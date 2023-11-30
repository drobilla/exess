// Copyright 2019-2021 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#include "result.h"

#include "exess/exess.h"

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

static ExessStatus
coerce_to_long(int64_t* const       out,
               const ExessDatatype  in_datatype,
               const void* const    in,
               const ExessCoercions coercions)
{
  switch (in_datatype) {
  case EXESS_NOTHING:
    break;

  case EXESS_BOOLEAN:
    *out = *(const bool*)in;
    return EXESS_SUCCESS;

  case EXESS_DECIMAL:
  case EXESS_DOUBLE: {
    const double d = *(const double*)in;

    if (!(coercions & (ExessCoercions)EXESS_ROUND) && d > trunc(d)) {
      return EXESS_WOULD_ROUND;
    }

    if (d < (double)-MAX_DOUBLE_INT || d > (double)MAX_DOUBLE_INT) {
      return EXESS_OUT_OF_RANGE;
    }

    *out = llrint(d);
    return EXESS_SUCCESS;
  }

  case EXESS_FLOAT: {
    const float f = *(const float*)in;

    if (!(coercions & (ExessCoercions)EXESS_ROUND) && f > truncf(f)) {
      return EXESS_WOULD_ROUND;
    }

    if (f < (float)-MAX_FLOAT_INT || f > (float)MAX_FLOAT_INT) {
      return EXESS_OUT_OF_RANGE;
    }

    *out = llrintf(f);
    return EXESS_SUCCESS;
  }

  case EXESS_INTEGER:
  case EXESS_NON_POSITIVE_INTEGER:
  case EXESS_NEGATIVE_INTEGER:
  case EXESS_LONG:
    *out = *(const int64_t*)in;
    return EXESS_SUCCESS;

  case EXESS_INT:
    *out = *(const int32_t*)in;
    return EXESS_SUCCESS;

  case EXESS_SHORT:
    *out = *(const int16_t*)in;
    return EXESS_SUCCESS;

  case EXESS_BYTE:
    *out = (int64_t) * (const int8_t*)in;
    return EXESS_SUCCESS;

  case EXESS_NON_NEGATIVE_INTEGER:
  case EXESS_ULONG: {
    const uint64_t u = *(const uint64_t*)in;

    if (u > INT64_MAX) {
      return EXESS_OUT_OF_RANGE;
    }

    *out = (int64_t)u;
    return EXESS_SUCCESS;
  }

  case EXESS_UINT:
    *out = *(const uint32_t*)in;
    return EXESS_SUCCESS;

  case EXESS_USHORT:
    *out = *(const uint16_t*)in;
    return EXESS_SUCCESS;

  case EXESS_UBYTE:
    *out = *(const uint8_t*)in;
    return EXESS_SUCCESS;

  case EXESS_POSITIVE_INTEGER: {
    const uint64_t u = *(const uint64_t*)in;

    if (u > INT64_MAX) {
      return EXESS_OUT_OF_RANGE;
    }

    *out = (int64_t)u;
    return EXESS_SUCCESS;
  }

  case EXESS_DURATION:
  case EXESS_DATETIME:
  case EXESS_TIME:
  case EXESS_DATE:
  case EXESS_HEX:
  case EXESS_BASE64:
    break;
  }

  return EXESS_UNSUPPORTED;
}

static ExessStatus
coerce_signed(int64_t* const      out,
              const ExessDatatype in_datatype,
              const void* const   in,
              const int64_t       min,
              const int64_t       max)
{
  const ExessStatus st = coerce_to_long(out, in_datatype, in, EXESS_LOSSLESS);

  return st                           ? st
         : (*out < min || *out > max) ? EXESS_OUT_OF_RANGE
                                      : EXESS_SUCCESS;
}

static ExessStatus
coerce_to_ulong(uint64_t* const      out,
                const ExessDatatype  in_datatype,
                const void* const    in,
                const ExessCoercions coercions)
{
  switch (in_datatype) {
  case EXESS_NOTHING:
    break;

  case EXESS_BOOLEAN:
    *out = *(const bool*)in;
    return EXESS_SUCCESS;

  case EXESS_DECIMAL:
  case EXESS_DOUBLE:
    if (!(coercions & (ExessCoercions)EXESS_ROUND) &&
        *(const double*)in > trunc(*(const double*)in)) {
      return EXESS_WOULD_ROUND;
    }

    if (*(const double*)in < 0.0 ||
        *(const double*)in > (double)MAX_DOUBLE_INT) {
      return EXESS_OUT_OF_RANGE;
    }

    *out = (uint64_t)llrint(*(const double*)in);
    return EXESS_SUCCESS;

  case EXESS_FLOAT:
    if (!(coercions & (ExessCoercions)EXESS_ROUND) &&
        *(const float*)in > truncf(*(const float*)in)) {
      return EXESS_WOULD_ROUND;
    }

    if (*(const float*)in < 0.0f || *(const float*)in > (float)MAX_FLOAT_INT) {
      return EXESS_OUT_OF_RANGE;
    }

    *out = (uint64_t)llrintf(*(const float*)in);
    return EXESS_SUCCESS;

  case EXESS_INTEGER:
  case EXESS_NON_POSITIVE_INTEGER:
  case EXESS_NEGATIVE_INTEGER:
  case EXESS_LONG:
    if (*(const int64_t*)in < 0) {
      return EXESS_OUT_OF_RANGE;
    }

    *out = (uint64_t) * (const int64_t*)in;
    return EXESS_SUCCESS;

  case EXESS_INT:
    if (*(const int32_t*)in < 0) {
      return EXESS_OUT_OF_RANGE;
    }

    *out = (uint64_t) * (const int*)in;
    return EXESS_SUCCESS;

  case EXESS_SHORT:
    if (*(const int16_t*)in < 0) {
      return EXESS_OUT_OF_RANGE;
    }

    *out = (uint64_t) * (const int16_t*)in;
    return EXESS_SUCCESS;

  case EXESS_BYTE:
    if (*(const int8_t*)in < 0) {
      return EXESS_OUT_OF_RANGE;
    }

    *out = (uint64_t) * (const int8_t*)in;
    return EXESS_SUCCESS;

  case EXESS_NON_NEGATIVE_INTEGER:
  case EXESS_ULONG:
    *out = *(const uint64_t*)in;
    return EXESS_SUCCESS;

  case EXESS_UINT:
    *out = *(const uint32_t*)in;
    return EXESS_SUCCESS;

  case EXESS_USHORT:
    *out = *(const uint16_t*)in;
    return EXESS_SUCCESS;

  case EXESS_UBYTE:
    *out = *(const uint8_t*)in;
    return EXESS_SUCCESS;

  case EXESS_POSITIVE_INTEGER:
    *out = *(const uint64_t*)in;
    return EXESS_SUCCESS;

  case EXESS_DURATION:
  case EXESS_DATETIME:
  case EXESS_TIME:
  case EXESS_DATE:
  case EXESS_HEX:
  case EXESS_BASE64:
    break;
  }

  return EXESS_UNSUPPORTED;
}

static ExessStatus
coerce_unsigned(uint64_t* const     out,
                const ExessDatatype in_datatype,
                const void* const   in,
                const uint64_t      max)
{
  const ExessStatus st = coerce_to_ulong(out, in_datatype, in, EXESS_LOSSLESS);

  return st ? st : (*out > max) ? EXESS_OUT_OF_RANGE : EXESS_SUCCESS;
}

static ExessResult
coerce_to_boolean(const ExessCoercions coercions,
                  const ExessDatatype  in_datatype,
                  const void* const    in,
                  bool* const          out)
{
  ExessStatus st    = EXESS_SUCCESS;
  int64_t     l_out = 0;

  if (!(st = coerce_to_long(&l_out, in_datatype, in, coercions))) {
    const bool truncate = (coercions & (ExessCoercions)EXESS_TRUNCATE);
    if (!truncate && l_out != 0 && l_out != 1) {
      return result(EXESS_WOULD_TRUNCATE, 0U);
    }

    *out = (l_out != 0);
    return result(EXESS_SUCCESS, sizeof(bool));
  }

  return result(st, 0U);
}

static ExessResult
coerce_to_decimal(const ExessDatatype in_datatype,
                  const void* const   in,
                  void* const         out)
{
  ExessStatus st    = EXESS_SUCCESS;
  int64_t     l_out = 0;

  switch (in_datatype) {
  case EXESS_DOUBLE:
    *(double*)out = *(const double*)in;
    return result(EXESS_SUCCESS, sizeof(double));

  case EXESS_FLOAT:
    *(double*)out = (double)*(const float*)in;
    return result(EXESS_SUCCESS, sizeof(double));

  default:
    if (!(st = coerce_signed(
            &l_out, in_datatype, in, -MAX_DOUBLE_INT, MAX_DOUBLE_INT))) {
      *(double*)out = (double)l_out;
      return result(EXESS_SUCCESS, sizeof(double));
    }
  }

  return result(st, 0U);
}

static ExessResult
coerce_to_double(const ExessDatatype in_datatype,
                 const void* const   in,
                 double* const       out)
{
  ExessStatus st    = EXESS_SUCCESS;
  int64_t     l_out = 0;

  switch (in_datatype) {
  case EXESS_DECIMAL:
    *out = *(const double*)in;
    return result(EXESS_SUCCESS, sizeof(double));

  case EXESS_FLOAT:
    *out = (double)*(const float*)in;
    return result(EXESS_SUCCESS, sizeof(double));

  default:
    if (!(st = coerce_signed(
            &l_out, in_datatype, in, -MAX_DOUBLE_INT, MAX_DOUBLE_INT))) {
      *out = (double)l_out;
      return result(EXESS_SUCCESS, sizeof(double));
    }
  }

  return result(st, 0U);
}

static ExessResult
coerce_to_float(const ExessCoercions coercions,
                const ExessDatatype  in_datatype,
                const void* const    in,
                float* const         out)
{
  ExessStatus st    = EXESS_SUCCESS;
  int64_t     l_out = 0;

  switch (in_datatype) {
  case EXESS_DECIMAL:
  case EXESS_DOUBLE:
    if (!(coercions & (ExessCoercions)EXESS_REDUCE_PRECISION)) {
      return result(EXESS_WOULD_REDUCE_PRECISION, 0U);
    }

    *out = (float)*(const double*)in;
    return result(EXESS_SUCCESS, sizeof(float));

  default:
    if (!(st = coerce_signed(
            &l_out, in_datatype, in, -MAX_FLOAT_INT, MAX_FLOAT_INT))) {
      *out = (float)l_out;
      return result(EXESS_SUCCESS, sizeof(float));
    }
  }

  return result(st, 0U);
}

static ExessResult
coerce_to_integer(const ExessCoercions coercions,
                  const ExessDatatype  in_datatype,
                  const void* const    in,
                  const int64_t        max,
                  int64_t* const       out)
{
  ExessStatus st    = EXESS_SUCCESS;
  int64_t     l_out = 0;

  if (!(st = coerce_to_long(&l_out, in_datatype, in, coercions))) {
    if (l_out > max) {
      return result(EXESS_OUT_OF_RANGE, 0U);
    }

    *out = l_out;
    return result(EXESS_SUCCESS, sizeof(int64_t));
  }

  return result(st, 0U);
}

static ExessResult
coerce_to_unsigned_integer(const ExessCoercions coercions,
                           const ExessDatatype  in_datatype,
                           const void* const    in,
                           const uint64_t       min,
                           uint64_t* const      out)
{
  ExessStatus st    = EXESS_SUCCESS;
  uint64_t    u_out = 0;

  if (!(st = coerce_to_ulong(&u_out, in_datatype, in, coercions))) {
    if (u_out < min) {
      return result(EXESS_OUT_OF_RANGE, 0U);
    }

    *out = u_out;
    return result(EXESS_SUCCESS, sizeof(uint64_t));
  }

  return result(st, 0U);
}

static ExessResult
coerce_to_time(const ExessCoercions coercions,
               const ExessDatatype  in_datatype,
               const void* const    in,
               ExessTime* const     out)
{
  if (in_datatype != EXESS_DATETIME) {
    return result(EXESS_UNSUPPORTED, 0U);
  }

  if (!(coercions & (ExessCoercions)EXESS_TRUNCATE)) {
    return result(EXESS_WOULD_TRUNCATE, 0U);
  }

  const ExessDateTime datetime = *(const ExessDateTime*)in;

  const ExessTime time = {datetime.is_utc ? EXESS_UTC : EXESS_LOCAL,
                          datetime.hour,
                          datetime.minute,
                          datetime.second,
                          datetime.nanosecond};

  *out = time;
  return result(EXESS_SUCCESS, sizeof(ExessTime));
}

static ExessResult
coerce_to_date(const ExessCoercions coercions,
               const ExessDatatype  in_datatype,
               const void* const    in,
               ExessDate* const     out)
{
  if (in_datatype != EXESS_DATETIME) {
    return result(EXESS_UNSUPPORTED, 0U);
  }

  if (!(coercions & (ExessCoercions)EXESS_TRUNCATE)) {
    return result(EXESS_WOULD_TRUNCATE, 0U);
  }

  const ExessDateTime datetime = *(const ExessDateTime*)in;

  const ExessDate date = {datetime.year,
                          datetime.month,
                          datetime.day,
                          datetime.is_utc ? EXESS_UTC : EXESS_LOCAL};

  *out = date;
  return result(EXESS_SUCCESS, sizeof(ExessDate));
}

ExessResult
exess_value_coerce(const ExessCoercions coercions,
                   const ExessDatatype  in_datatype,
                   const size_t         in_size,
                   const void* const    in,
                   const ExessDatatype  out_datatype,
                   const size_t         out_size,
                   void* const          out)
{
  // Ensure the input is sufficiently large so we don't read out of bounds
  if (in_size < exess_value_size(in_datatype)) {
    return result(EXESS_BAD_VALUE, 0U);
  }

  // Copy value verbatim for trivial conversions
  if ((out_datatype == in_datatype) ||
      (out_datatype == EXESS_HEX && in_datatype == EXESS_BASE64) ||
      (out_datatype == EXESS_BASE64 && in_datatype == EXESS_HEX)) {
    if (out_size < in_size) {
      return result(EXESS_NO_SPACE, 0U);
    }

    memcpy(out, in, in_size);
    return result(EXESS_SUCCESS, in_size);
  }

  // Ensure the output is sufficiently large so we don't write out of bounds
  if (out_size < exess_value_size(out_datatype)) {
    return result(EXESS_NO_SPACE, 0U);
  }

  ExessStatus st    = EXESS_UNSUPPORTED;
  int64_t     l_out = 0;
  uint64_t    u_out = 0U;

  switch (out_datatype) {
  case EXESS_NOTHING:
    break;

  case EXESS_BOOLEAN:
    return coerce_to_boolean(coercions, in_datatype, in, (bool*)out);
  case EXESS_DECIMAL:
    return coerce_to_decimal(in_datatype, in, (double*)out);
  case EXESS_DOUBLE:
    return coerce_to_double(in_datatype, in, (double*)out);
  case EXESS_FLOAT:
    return coerce_to_float(coercions, in_datatype, in, (float*)out);

  case EXESS_INTEGER:
    return coerce_to_integer(
      coercions, in_datatype, in, INT64_MAX, (int64_t*)out);

  case EXESS_NON_POSITIVE_INTEGER:
    return coerce_to_integer(coercions, in_datatype, in, 0, (int64_t*)out);

  case EXESS_NEGATIVE_INTEGER:
    return coerce_to_integer(coercions, in_datatype, in, -1, (int64_t*)out);

  case EXESS_LONG:
    return coerce_to_integer(
      coercions, in_datatype, in, INT64_MAX, (int64_t*)out);

  case EXESS_INT:
    if (!(st = coerce_signed(&l_out, in_datatype, in, INT32_MIN, INT32_MAX))) {
      *(int32_t*)out = (int32_t)l_out;
      return result(EXESS_SUCCESS, sizeof(int32_t));
    }
    break;

  case EXESS_SHORT:
    if (!(st = coerce_signed(&l_out, in_datatype, in, INT16_MIN, INT16_MAX))) {
      *(int16_t*)out = (int16_t)l_out;
      return result(EXESS_SUCCESS, sizeof(int16_t));
    }
    break;

  case EXESS_BYTE:
    if (!(st = coerce_signed(&l_out, in_datatype, in, INT8_MIN, INT8_MAX))) {
      *(int8_t*)out = (int8_t)l_out;
      return result(EXESS_SUCCESS, sizeof(int8_t));
    }
    break;

  case EXESS_NON_NEGATIVE_INTEGER:
  case EXESS_ULONG:
    return coerce_to_unsigned_integer(
      coercions, in_datatype, in, 0U, (uint64_t*)out);

  case EXESS_UINT:
    if (!(st = coerce_unsigned(&u_out, in_datatype, in, UINT32_MAX))) {
      *(uint32_t*)out = (uint32_t)u_out;
      return result(EXESS_SUCCESS, sizeof(uint32_t));
    }
    break;

  case EXESS_USHORT:
    if (!(st = coerce_unsigned(&u_out, in_datatype, in, UINT16_MAX))) {
      *(uint16_t*)out = (uint16_t)u_out;
      return result(EXESS_SUCCESS, sizeof(uint16_t));
    }
    break;

  case EXESS_UBYTE:
    if (!(st = coerce_unsigned(&u_out, in_datatype, in, UINT8_MAX))) {
      *(uint8_t*)out = (uint8_t)u_out;
      return result(EXESS_SUCCESS, sizeof(uint8_t));
    }
    break;

  case EXESS_POSITIVE_INTEGER:
    return coerce_to_unsigned_integer(
      coercions, in_datatype, in, 1U, (uint64_t*)out);

  case EXESS_DURATION:
  case EXESS_DATETIME:
    break;

  case EXESS_TIME:
    return coerce_to_time(coercions, in_datatype, in, (ExessTime*)out);
  case EXESS_DATE:
    return coerce_to_date(coercions, in_datatype, in, (ExessDate*)out);

  case EXESS_HEX:
  case EXESS_BASE64:
    break;
  }

  return result(st, 0U);
}
