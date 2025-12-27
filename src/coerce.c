// Copyright 2019-2025 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#include "result.h"

#include <exess/exess.h>

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

  case EXESS_DECIMAL:
  case EXESS_DOUBLE:
    return FP_CONVERSION(
      double, *(const double*)in, MAX_DOUBLE_INT, trunc, llrint);
  case EXESS_FLOAT:
    return FP_CONVERSION(
      float, *(const float*)in, MAX_FLOAT_INT, truncf, llrintf);

  case EXESS_INTEGER:
  case EXESS_NON_POSITIVE_INTEGER:
  case EXESS_NEGATIVE_INTEGER:
  case EXESS_LONG:
    return SCONVERSION(EXESS_SUCCESS, *(const int64_t*)in);
  case EXESS_INT:
    return SCONVERSION(EXESS_SUCCESS, *(const int32_t*)in);
  case EXESS_SHORT:
    return SCONVERSION(EXESS_SUCCESS, *(const int16_t*)in);
  case EXESS_BYTE:
    return SCONVERSION(EXESS_SUCCESS, *(const int8_t*)in);

  case EXESS_NON_NEGATIVE_INTEGER:
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

  case EXESS_POSITIVE_INTEGER: {
    const uint64_t u = *(const uint64_t*)in;
    return (u > INT64_MAX) ? SCONVERSION(EXESS_OUT_OF_RANGE, 0)
                           : SCONVERSION(EXESS_SUCCESS, (int64_t)u);
  }

  case EXESS_DURATION:
  case EXESS_DATE_TIME:
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
  if (in_datatype == EXESS_NON_NEGATIVE_INTEGER || in_datatype == EXESS_ULONG ||
      in_datatype == EXESS_POSITIVE_INTEGER) {
    return UCONVERSION(EXESS_SUCCESS, *(const uint64_t*)in);
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
                       const uint64_t       min,
                       const uint64_t       max)
{
  const UConversion c = convert_unsigned(coercions, in_datatype, in);

  return (!c.status && (c.value < min || c.value > max))
           ? UCONVERSION(EXESS_OUT_OF_RANGE, 0U)
           : c;
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
to_double(const ExessDatatype in_datatype,
          const void* const   in,
          double* const       out)
{
  if (in_datatype == EXESS_DECIMAL) {
    *out = *(const double*)in;
    return RESULT(EXESS_SUCCESS, sizeof(double));
  }

  if (in_datatype == EXESS_FLOAT) {
    *out = (double)*(const float*)in;
    return RESULT(EXESS_SUCCESS, sizeof(double));
  }

  const SConversion c =
    convert_signed_range(0U, in_datatype, in, -MAX_DOUBLE_INT, MAX_DOUBLE_INT);

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
  if (in_datatype == EXESS_DECIMAL || in_datatype == EXESS_DOUBLE) {
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
to_long(const ExessCoercions coercions,
        const ExessDatatype  in_datatype,
        const void* const    in,
        const int64_t        max,
        int64_t* const       out)
{
  const SConversion c =
    convert_signed_range(coercions, in_datatype, in, INT64_MIN, max);

  ENSURE(!c.status, c.status);
  *out = c.value;
  return RESULT(EXESS_SUCCESS, sizeof(int64_t));
}

static ExessResult
to_ulong(const ExessCoercions coercions,
         const ExessDatatype  in_datatype,
         const void* const    in,
         const uint64_t       min,
         uint64_t* const      out)
{
  const UConversion c =
    convert_unsigned_range(coercions, in_datatype, in, min, UINT64_MAX);
  ENSURE(!c.status, c.status);

  *out = c.value;
  return RESULT(EXESS_SUCCESS, sizeof(uint64_t));
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
  ENSURE(out_size >= exess_value_sizes[out_datatype], EXESS_NO_SPACE);

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
  case EXESS_DECIMAL:
  case EXESS_DOUBLE:
    return to_double(in_datatype, in, (double*)out);
  case EXESS_FLOAT:
    return to_float(coercions, in_datatype, in, (float*)out);

  case EXESS_INTEGER:
    return to_long(coercions, in_datatype, in, INT64_MAX, (int64_t*)out);

  case EXESS_NON_POSITIVE_INTEGER:
    return to_long(coercions, in_datatype, in, 0, (int64_t*)out);

  case EXESS_NEGATIVE_INTEGER:
    return to_long(coercions, in_datatype, in, -1, (int64_t*)out);

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

  case EXESS_NON_NEGATIVE_INTEGER:
  case EXESS_ULONG:
    FIXED_CASE(
      uint64_t,
      convert_unsigned_range(coercions, in_datatype, in, 0U, UINT64_MAX));
  case EXESS_UINT:
    FIXED_CASE(
      uint32_t,
      convert_unsigned_range(coercions, in_datatype, in, 0U, UINT32_MAX));
  case EXESS_USHORT:
    FIXED_CASE(
      uint16_t,
      convert_unsigned_range(coercions, in_datatype, in, 0U, UINT16_MAX));
  case EXESS_UBYTE:
    FIXED_CASE(
      uint8_t,
      convert_unsigned_range(coercions, in_datatype, in, 0U, UINT8_MAX));

  case EXESS_POSITIVE_INTEGER:
    return to_ulong(coercions, in_datatype, in, 1U, (uint64_t*)out);

  case EXESS_DURATION:
  case EXESS_DATE_TIME:
    break;

  case EXESS_TIME:
    return to_time(coercions, in_datatype, in, (ExessTime*)out);
  case EXESS_DATE:
    return to_date(coercions, in_datatype, in, (ExessDate*)out);

  case EXESS_HEX:
  case EXESS_BASE64:
    break;
  }

  return RESULT(EXESS_UNSUPPORTED, 0U);
}
