// Copyright 2019-2025 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#include "big_decimal.h"
#include "decimal_to_double.h"
#include "double_to_decimal.h"
#include "macros.h"
#include "number_utils.h"
#include "result.h"

#include <exess/exess.h>

#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

/* Limits for the range of integers that can be exactly represented in floating
   point types.  Note that these limits are one less than the largest value,
   since values larger than that may round to it which causes problems with
   perfect round-tripping.  For example, 16777217 when parsed as a float will
   result in 1.6777216E7, which a "lossless" coercion would then convert to
   16777216. */

#define MAX_FLOAT_INT 16777215
#define MAX_DOUBLE_INT 9007199254740991L

#ifndef FLT_DECIMAL_DIG
#  define FLT_DECIMAL_DIG 9U
#endif
#ifndef DBL_DECIMAL_DIG
#  define DBL_DECIMAL_DIG 17U
#endif

#define ENSURE(condition, return_status_otherwise)  \
  do {                                              \
    if (!(condition)) {                             \
      return RESULT((return_status_otherwise), 0U); \
    }                                               \
  } while (0)

#define ENSURE_COERCION(coercions, coercion) \
  ENSURE(((coercions) & (ExessCoercions)(coercion)), EXESS_LOSS)

#ifdef __cplusplus
#  define UCONVERSION(s, v) (UConversion{s, v})
#  define SCONVERSION(s, v) (SConversion{s, v})
#else
#  define UCONVERSION(s, v) ((UConversion){s, v})
#  define SCONVERSION(s, v) ((SConversion){s, v})
#endif

typedef struct {
  ExessStatus status;
  int64_t     value;
} SConversion;

typedef struct {
  ExessStatus status;
  uint64_t    value;
} UConversion;

static bool
round_decimal_up(const BigDecimal* const d, const char* const digits)
{
  if (digits[d->expt] > '5') {
    return true;
  }

  if (digits[d->expt] < '5') {
    return false;
  }

  assert(d->n_digits > 0);
  const char ones = (char)((d->expt > 0) ? digits[d->expt - 1] : '0');
  return ((ones - '0') % 2);
}

static UConversion
convert_significand(const ExessCoercions coercions, const void* const in)
{
  BigDecimal        d      = *(const BigDecimal*)in;
  const char* const digits = (const char*)in + sizeof(d);

  bool round_up = false;
  if (d.expt < (int)d.n_digits) {
    if (coercions & (ExessCoercions)EXESS_TRUNCATE) {
      d.n_digits = (uint8_t)MAX(0, d.expt);
    } else if (coercions & (ExessCoercions)EXESS_ROUND) {
      if (d.expt <= 0) {
        return UCONVERSION(EXESS_SUCCESS, 0U);
      }

      round_up   = round_decimal_up(&d, digits);
      d.n_digits = (uint8_t)d.expt;
    } else {
      return UCONVERSION(EXESS_LOSS, 0U);
    }
  }

  uint64_t magnitude = 0U;
  size_t   i         = 0U;
  for (; i < d.n_digits; ++i) {
    const uint64_t next = (magnitude * 10U) + (uint64_t)(digits[i] - '0');
    if (next < magnitude) {
      return UCONVERSION(EXESS_OUT_OF_RANGE, i);
    }

    magnitude = next;
  }

  return UCONVERSION(EXESS_SUCCESS, magnitude + round_up);
}

static SConversion
convert_signed_decimal(const ExessCoercions coercions, const void* const in)
{
  BigDecimal        d         = *(const BigDecimal*)in;
  const UConversion c         = convert_significand(coercions, in);
  const uint64_t    magnitude = c.value;

  return c.status ? SCONVERSION(c.status, 0U)
         : (d.expt == 19 &&
            (d.kind == EXESS_NEGATIVE && magnitude == 9223372036854775808ULL))
           ? SCONVERSION(c.status, INT64_MIN)
         : (d.expt > 19 ||
            (d.expt == 19 && (magnitude >= 9223372036854775808ULL)))
           ? SCONVERSION(EXESS_OUT_OF_RANGE, 0)
         : (d.kind == EXESS_NEGATIVE)
           ? SCONVERSION(c.status, -(int64_t)magnitude)
           : SCONVERSION(c.status, (int64_t)magnitude);
}

static SConversion
convert_signed(const ExessCoercions coercions,
               const ExessDatatype  in_datatype,
               const void* const    in)
{
#define FP_CONVERSION(Type, f, max, trunc_func, round_func)           \
  ((f) < (Type)(-(max)) || (f) > (Type)(max))                         \
    ? SCONVERSION(EXESS_OUT_OF_RANGE, 0)                              \
  : ((f) == trunc_func(f)) ? SCONVERSION(EXESS_SUCCESS, (int64_t)(f)) \
  : (coercions & (ExessCoercions)EXESS_ROUND)                         \
    ? SCONVERSION(EXESS_SUCCESS, round_func(f))                       \
  : (coercions & (ExessCoercions)EXESS_TRUNCATE)                      \
    ? SCONVERSION(EXESS_SUCCESS, (int64_t)(f))                        \
    : SCONVERSION(EXESS_LOSS, 0)

  switch (in_datatype) {
  case EXESS_NOTHING:
    break;

  case EXESS_BOOLEAN:
    return SCONVERSION(EXESS_SUCCESS, *(const bool*)in);

  case EXESS_DOUBLE:
    return FP_CONVERSION(
      double, *(const double*)in, MAX_DOUBLE_INT, trunc, llrint);
  case EXESS_FLOAT:
    return FP_CONVERSION(
      float, *(const float*)in, MAX_FLOAT_INT, truncf, llrintf);

  case EXESS_LONG:
    return SCONVERSION(EXESS_SUCCESS, *(const int64_t*)in);
  case EXESS_INT:
    return SCONVERSION(EXESS_SUCCESS, *(const int32_t*)in);
  case EXESS_SHORT:
    return SCONVERSION(EXESS_SUCCESS, *(const int16_t*)in);
  case EXESS_BYTE:
    return SCONVERSION(EXESS_SUCCESS, *(const int8_t*)in);

  case EXESS_ULONG: {
    const uint64_t u = *(const uint64_t*)in;
    return (u > INT64_MAX) ? SCONVERSION(EXESS_OUT_OF_RANGE, 0)
                           : SCONVERSION(EXESS_SUCCESS, (int64_t)u);
  }

  case EXESS_UINT:
    return SCONVERSION(EXESS_SUCCESS, *(const uint32_t*)in);
  case EXESS_USHORT:
    return SCONVERSION(EXESS_SUCCESS, *(const uint16_t*)in);
  case EXESS_UBYTE:
    return SCONVERSION(EXESS_SUCCESS, *(const uint8_t*)in);

  case EXESS_DECIMAL:
  case EXESS_INTEGER:
  case EXESS_NON_POSITIVE_INTEGER:
  case EXESS_NEGATIVE_INTEGER:
  case EXESS_NON_NEGATIVE_INTEGER:
  case EXESS_POSITIVE_INTEGER:
    return convert_signed_decimal(coercions, in);

  case EXESS_DURATION:
  case EXESS_YEAR_MONTH_DURATION:
  case EXESS_DAY_TIME_DURATION:
  case EXESS_DATE_TIME:
  case EXESS_DATE_TIME_STAMP:
  case EXESS_TIME:
  case EXESS_DATE:
  case EXESS_HEX:
  case EXESS_BASE64:
    break;
  }

  return SCONVERSION(EXESS_UNSUPPORTED, 0);
}

static UConversion
convert_unsigned(const ExessCoercions coercions,
                 const ExessDatatype  in_datatype,
                 const void* const    in)
{
  if (in_datatype == EXESS_ULONG) {
    return UCONVERSION(EXESS_SUCCESS, *(const uint64_t*)in);
  }

  if (in_datatype >= EXESS_DECIMAL && in_datatype <= EXESS_POSITIVE_INTEGER) {
    BigDecimal        d = *(const BigDecimal*)in;
    const UConversion c = convert_significand(coercions, in);

    return c.status        ? UCONVERSION(c.status, 0U)
           : (d.expt > 20) ? UCONVERSION(EXESS_OUT_OF_RANGE, 0U)
                           : c;
  }

  const SConversion c = convert_signed(coercions, in_datatype, in);

  return c.status        ? UCONVERSION(c.status, 0U)
         : (c.value < 0) ? UCONVERSION(EXESS_OUT_OF_RANGE, 0U)
                         : UCONVERSION(EXESS_SUCCESS, (uint64_t)c.value);
}

static SConversion
convert_signed_range(const ExessCoercions coercions,
                     const ExessDatatype  in_datatype,
                     const void* const    in,
                     const int64_t        min,
                     const int64_t        max)
{
  const SConversion c = convert_signed(coercions, in_datatype, in);

  return (!c.status && (c.value < min || c.value > max))
           ? SCONVERSION(EXESS_OUT_OF_RANGE, 0U)
           : c;
}

static UConversion
convert_unsigned_range(const ExessCoercions coercions,
                       const ExessDatatype  in_datatype,
                       const void* const    in,
                       const uint64_t       max)
{
  const UConversion c = convert_unsigned(coercions, in_datatype, in);

  return (!c.status && c.value > max) ? UCONVERSION(EXESS_OUT_OF_RANGE, 0U) : c;
}

static ExessResult
to_boolean(const ExessCoercions coercions,
           const ExessDatatype  in_datatype,
           const void* const    in,
           bool* const          out)
{
  const SConversion c = convert_signed(coercions, in_datatype, in);
  ENSURE(!c.status, c.status);
  ENSURE((coercions & (ExessCoercions)EXESS_TRUNCATE) || c.value == 0 ||
           c.value == 1,
         EXESS_LOSS);

  *out = !!c.value;
  return RESULT(EXESS_SUCCESS, sizeof(bool));
}

static ExessResult
to_decimal(const ExessCoercions coercions,
           const ExessDatatype  in_datatype,
           const void* const    in,
           const size_t         out_size,
           void* const          out)
{
  BigDecimal* const head        = (BigDecimal*)out;
  const size_t      digits_size = out_size - sizeof(*head);
  char* const       digits      = (char*)(head + 1U);

  if (in_datatype == EXESS_DECIMAL) {
    const BigDecimal* const in_head = (const BigDecimal*)in;
    const size_t            size = sizeof(BigDecimal) + in_head->n_digits + 1;
    if (out_size < size) {
      return RESULT(EXESS_NO_SPACE, size);
    }

    memcpy(out, in, size);
    return RESULT(EXESS_SUCCESS, size);
  }

  if (in_datatype == EXESS_DOUBLE || in_datatype == EXESS_FLOAT) {
    const double d = (in_datatype == EXESS_DOUBLE) ? *(const double*)in
                                                   : (double)*(const float*)in;

    *head = double_to_decimal(d, digits_size, digits);
    if (head->n_digits == digits_size) { // No space for terminator
      return RESULT(EXESS_NO_SPACE, sizeof(*head) + head->n_digits + 1U);
    }

    digits[head->n_digits] = '\0';
    return RESULT(EXESS_SUCCESS, sizeof(*head) + head->n_digits + 1U);
  }

  if (in_datatype == EXESS_ULONG) {
    const uint64_t    u_in = *(const uint64_t*)in;
    const ExessResult r    = write_digits(u_in, digits_size, digits, 0);
    head->kind             = u_in ? EXESS_POSITIVE : EXESS_POSITIVE_ZERO;
    head->n_digits         = (uint8_t)r.count;
    head->expt             = (int16_t)r.count;
    digits[head->n_digits] = '\0';
    return RESULT(r.status, sizeof(*head) + r.count + 1U);
  }

  const SConversion c =
    convert_signed_range(coercions, in_datatype, in, INT64_MIN, INT64_MAX);

  ENSURE(!c.status, c.status);
  if (!c.value) {
    head->kind     = EXESS_POSITIVE_ZERO;
    head->n_digits = 0U;
    head->expt     = 0;
    digits[0]      = '\0';
    return RESULT(EXESS_SUCCESS, sizeof(*head) + 1U);
  }

  if (c.value == INT64_MIN) {
    if (out_size < sizeof(BigDecimal) + 20U) {
      return RESULT(EXESS_NO_SPACE, sizeof(BigDecimal) + head->n_digits + 1);
    }

    head->kind     = EXESS_NEGATIVE;
    head->n_digits = 19U;
    head->expt     = 19;
    memcpy(digits, "9223372036854775808", 19U);
    return RESULT(EXESS_SUCCESS, sizeof(*head) + 20U);
  }

  uint64_t abs_value   = c.value < 0 ? (uint64_t)-c.value : (uint64_t)c.value;
  int      expt_offset = 0;
  while (abs_value >= 10U && (abs_value % 10U) == 0U) {
    abs_value /= 10U;
    ++expt_offset;
  }

  const ExessResult r = write_digits(abs_value, digits_size - 1U, digits, 0);

  head->kind             = c.value < 0 ? EXESS_NEGATIVE : EXESS_POSITIVE;
  head->n_digits         = (uint8_t)r.count;
  head->expt             = (int16_t)(expt_offset + (int)r.count);
  digits[head->n_digits] = '\0';

  return RESULT(r.status, sizeof(*head) + r.count + 1U);
}

static ExessResult
to_double(const ExessCoercions coercions,
          const ExessDatatype  in_datatype,
          const void* const    in,
          double* const        out)
{
  if (in_datatype == EXESS_DECIMAL) {
    const BigDecimal  d      = *(const BigDecimal*)in;
    const char* const digits = (const char*)in + sizeof(d);
    ENSURE((coercions & (ExessCoercions)EXESS_APPROXIMATE) ||
             d.n_digits <= DBL_DECIMAL_DIG,
           EXESS_LOSS);

    *out = decimal_to_double(d, digits);
    return RESULT(EXESS_SUCCESS, sizeof(double));
  }

  if (in_datatype == EXESS_FLOAT) {
    *out = (double)*(const float*)in;
    return RESULT(EXESS_SUCCESS, sizeof(double));
  }

  const SConversion c = convert_signed_range(
    coercions, in_datatype, in, -MAX_DOUBLE_INT, MAX_DOUBLE_INT);

  ENSURE(!c.status, c.status);
  *out = (double)c.value;
  return RESULT(EXESS_SUCCESS, sizeof(double));
}

static ExessResult
to_float(const ExessCoercions coercions,
         const ExessDatatype  in_datatype,
         const void* const    in,
         float* const         out)
{
  if (in_datatype == EXESS_DECIMAL) {
    const BigDecimal  d      = *(const BigDecimal*)in;
    const char* const digits = (const char*)in + sizeof(d);
    ENSURE((coercions & (ExessCoercions)EXESS_APPROXIMATE) ||
             d.n_digits <= FLT_DECIMAL_DIG,
           EXESS_LOSS);

    *out = (float)decimal_to_double(d, digits);
    return RESULT(EXESS_SUCCESS, sizeof(float));
  }

  if (in_datatype == EXESS_DOUBLE) {
    ENSURE_COERCION(coercions, EXESS_APPROXIMATE);
    *out = (float)*(const double*)in;
    return RESULT(EXESS_SUCCESS, sizeof(float));
  }

  const SConversion c = convert_signed_range(
    coercions, in_datatype, in, -MAX_FLOAT_INT, MAX_FLOAT_INT);

  ENSURE(!c.status, c.status);
  *out = (float)c.value;
  return RESULT(EXESS_SUCCESS, sizeof(float));
}

static ExessResult
to_integer(const ExessCoercions coercions,
           const ExessDatatype  in_datatype,
           const void* const    in,
           const size_t         out_size,
           void* const          out)
{
  ExessResult r = to_decimal(coercions, in_datatype, in, out_size, out);
  if (r.status) {
    return r;
  }

  BigDecimal* const d = (BigDecimal*)out;
  if (d->expt < (int)d->n_digits) {
    char* const digits = (char*)(d + 1);
    if (coercions & (ExessCoercions)EXESS_TRUNCATE) {
      d->n_digits         = (uint8_t)MAX(0, d->expt);
      digits[d->n_digits] = '\0';
      if (!d->n_digits) {
        d->kind = EXESS_POSITIVE_ZERO;
        d->expt = 0;
      }
    } else if (coercions & (ExessCoercions)EXESS_ROUND) {
      const bool up = round_decimal_up(d, digits);
      d->n_digits   = (uint8_t)MAX(0, d->expt);
      if (up) {
        ++digits[d->n_digits - 1];
      }
      if (!d->n_digits) {
        d->kind = EXESS_POSITIVE_ZERO;
        d->expt = 0;
      }
    } else {
      r.status = EXESS_LOSS;
    }
  }

  return r;
}

static ExessResult
to_constrained_integer(const ExessCoercions coercions,
                       const ExessDatatype  in_datatype,
                       const void* const    in,
                       const ExessDatatype  out_datatype,
                       const size_t         out_size,
                       void* const          out)
{
  const ExessResult r = to_integer(coercions, in_datatype, in, out_size, out);
  ENSURE(!r.status, r.status);

  const BigDecimalKind kind = ((BigDecimal*)out)->kind;

  const bool bad_kind =
    (out_datatype == EXESS_NON_POSITIVE_INTEGER)   ? (kind == EXESS_POSITIVE)
    : (out_datatype == EXESS_NEGATIVE_INTEGER)     ? (kind != EXESS_NEGATIVE)
    : (out_datatype == EXESS_NON_NEGATIVE_INTEGER) ? (kind == EXESS_NEGATIVE)
                                                   : (kind != EXESS_POSITIVE);

  return bad_kind ? RESULT(EXESS_OUT_OF_RANGE, 0U) : r;
}

static ExessResult
to_time(const ExessCoercions coercions,
        const ExessDatatype  in_datatype,
        const void* const    in,
        ExessTime* const     out)
{
  ENSURE(in_datatype == EXESS_DATE_TIME, EXESS_UNSUPPORTED);
  ENSURE_COERCION(coercions, EXESS_TRUNCATE);

  const ExessDateTime datetime = *(const ExessDateTime*)in;

  out->zone       = datetime.zone;
  out->hour       = datetime.hour;
  out->minute     = datetime.minute;
  out->second     = datetime.second;
  out->nanosecond = datetime.nanosecond;
  return RESULT(EXESS_SUCCESS, sizeof(ExessTime));
}

static ExessResult
to_date(const ExessCoercions coercions,
        const ExessDatatype  in_datatype,
        const void* const    in,
        ExessDate* const     out)
{
  ENSURE(in_datatype == EXESS_DATE_TIME, EXESS_UNSUPPORTED);
  ENSURE_COERCION(coercions, EXESS_TRUNCATE);

  const ExessDateTime datetime = *(const ExessDateTime*)in;

  out->year  = datetime.year;
  out->month = datetime.month;
  out->day   = datetime.day;
  out->zone  = datetime.zone;
  return RESULT(EXESS_SUCCESS, sizeof(ExessDate));
}

static uint8_t
min_value_size(const ExessDatatype datatype)
{
  return (datatype >= EXESS_DECIMAL && datatype <= EXESS_POSITIVE_INTEGER)
           ? 6U
           : exess_value_sizes[datatype];
}

ExessResult
exess_coerce_value(const ExessCoercions coercions,
                   const ExessDatatype  in_datatype,
                   const size_t         in_size,
                   const void* const    in,
                   const ExessDatatype  out_datatype,
                   const size_t         out_size,
                   void* const          out)
{
  // Ensure the input and output are large enough (for fixed types)
  ENSURE(in_size >= exess_value_sizes[in_datatype], EXESS_BAD_VALUE);
  ENSURE(out_size >= min_value_size(out_datatype), EXESS_NO_SPACE);

  // Copy value verbatim for trivial conversions
  if ((out_datatype == in_datatype) ||
      (out_datatype == EXESS_HEX && in_datatype == EXESS_BASE64) ||
      (out_datatype == EXESS_BASE64 && in_datatype == EXESS_HEX)) {
    ENSURE(out_size >= in_size, EXESS_NO_SPACE);
    memcpy(out, in, in_size);
    return RESULT(EXESS_SUCCESS, in_size);
  }

#define FIXED_CASE(Type, c)                                    \
  do {                                                         \
    *(Type*)out = (c).status ? (Type)0 : (Type)(c).value;      \
    return RESULT((c).status, (c).status ? 0U : sizeof(Type)); \
  } while (0)

  switch (out_datatype) {
  case EXESS_NOTHING:
    break;

  case EXESS_BOOLEAN:
    return to_boolean(coercions, in_datatype, in, (bool*)out);
  case EXESS_DOUBLE:
    return to_double(coercions, in_datatype, in, (double*)out);
  case EXESS_FLOAT:
    return to_float(coercions, in_datatype, in, (float*)out);

  case EXESS_LONG:
    FIXED_CASE(
      int64_t,
      convert_signed_range(coercions, in_datatype, in, INT64_MIN, INT64_MAX));
  case EXESS_INT:
    FIXED_CASE(
      int32_t,
      convert_signed_range(coercions, in_datatype, in, INT32_MIN, INT32_MAX));
  case EXESS_SHORT:
    FIXED_CASE(
      int16_t,
      convert_signed_range(coercions, in_datatype, in, INT16_MIN, INT16_MAX));
  case EXESS_BYTE:
    FIXED_CASE(
      int8_t,
      convert_signed_range(coercions, in_datatype, in, INT8_MIN, INT8_MAX));

  case EXESS_ULONG:
    FIXED_CASE(uint64_t,
               convert_unsigned_range(coercions, in_datatype, in, UINT64_MAX));
  case EXESS_UINT:
    FIXED_CASE(uint32_t,
               convert_unsigned_range(coercions, in_datatype, in, UINT32_MAX));
  case EXESS_USHORT:
    FIXED_CASE(uint16_t,
               convert_unsigned_range(coercions, in_datatype, in, UINT16_MAX));
  case EXESS_UBYTE:
    FIXED_CASE(uint8_t,
               convert_unsigned_range(coercions, in_datatype, in, UINT8_MAX));

  case EXESS_DECIMAL:
    return to_decimal(coercions, in_datatype, in, out_size, out);
  case EXESS_INTEGER:
    return to_integer(coercions, in_datatype, in, out_size, out);
  case EXESS_NON_POSITIVE_INTEGER:
  case EXESS_NEGATIVE_INTEGER:
  case EXESS_NON_NEGATIVE_INTEGER:
  case EXESS_POSITIVE_INTEGER:
    return to_constrained_integer(
      coercions, in_datatype, in, out_datatype, out_size, out);

  case EXESS_DURATION:
  case EXESS_YEAR_MONTH_DURATION:
  case EXESS_DAY_TIME_DURATION:
  case EXESS_DATE_TIME:
  case EXESS_DATE_TIME_STAMP:
    break;

  case EXESS_TIME:
    return to_time(coercions, in_datatype, in, (ExessTime*)out);
  case EXESS_DATE:
    return to_date(coercions, in_datatype, in, (ExessDate*)out);

  case EXESS_HEX:
  case EXESS_BASE64:
    break;
  }

#undef FIXED_CASE

  return RESULT(EXESS_UNSUPPORTED, 0U);
}
