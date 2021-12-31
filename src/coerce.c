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
coerce_to_long(int64_t* const           out,
               const ExessDatatype      in_datatype,
               const void* const        in,
               const ExessCoercionFlags coercions)
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

    if (!(coercions & (ExessCoercionFlags)EXESS_ROUND) && d > trunc(d)) {
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

    if (!(coercions & (ExessCoercionFlags)EXESS_ROUND) && f > truncf(f)) {
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
coerce_to_ulong(uint64_t* const          out,
                const ExessDatatype      in_datatype,
                const void* const        in,
                const ExessCoercionFlags coercions)
{
  switch (in_datatype) {
  case EXESS_NOTHING:
    break;

  case EXESS_BOOLEAN:
    *out = *(const bool*)in;
    return EXESS_SUCCESS;

  case EXESS_DECIMAL:
  case EXESS_DOUBLE:
    if (!(coercions & (ExessCoercionFlags)EXESS_ROUND) &&
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
    if (!(coercions & (ExessCoercionFlags)EXESS_ROUND) &&
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

ExessResult
exess_coerce_value(const ExessCoercionFlags coercions,
                   const ExessDatatype      in_datatype,
                   const size_t             in_size,
                   const void* const        in,
                   const ExessDatatype      out_datatype,
                   const size_t             out_size,
                   void* const              out)
{
  // Ensure the input is sufficiently large so we don't read out of bounds
  if (in_size < exess_value_size(in_datatype)) {
    return result(EXESS_BAD_VALUE, 0u);
  }

  // Copy value verbatim for trivial conversions
  if ((out_datatype == in_datatype) ||
      (out_datatype == EXESS_HEX && in_datatype == EXESS_BASE64) ||
      (out_datatype == EXESS_BASE64 && in_datatype == EXESS_HEX)) {
    if (out_size < in_size) {
      return result(EXESS_NO_SPACE, 0u);
    }

    memcpy(out, in, in_size);
    return result(EXESS_SUCCESS, in_size);
  }

  // Ensure the output is sufficiently large so we don't write out of bounds
  if (out_size < exess_value_size(out_datatype)) {
    return result(EXESS_NO_SPACE, 0u);
  }

  ExessStatus st    = EXESS_UNSUPPORTED;
  int64_t     l_out = 0;
  uint64_t    u_out = 0u;

  switch (out_datatype) {
  case EXESS_NOTHING:
    break;

  case EXESS_BOOLEAN:
    if (!(st = coerce_to_long(&l_out, in_datatype, in, coercions))) {
      const bool truncate = (coercions & (ExessCoercionFlags)EXESS_TRUNCATE);
      if (!truncate && l_out != 0 && l_out != 1) {
        return result(EXESS_WOULD_TRUNCATE, 0u);
      }

      *(bool*)out = (l_out != 0);
      return result(EXESS_SUCCESS, sizeof(bool));
    }

    break;

  case EXESS_DECIMAL:
    if (in_datatype == EXESS_DOUBLE) {
      *(double*)out = *(const double*)in;
      return result(EXESS_SUCCESS, sizeof(double));
    }

    if (in_datatype == EXESS_FLOAT) {
      *(double*)out = (double)*(const float*)in;
      return result(EXESS_SUCCESS, sizeof(double));
    }

    if (!(st = coerce_signed(
            &l_out, in_datatype, in, -MAX_DOUBLE_INT, MAX_DOUBLE_INT))) {
      *(double*)out = (double)l_out;
      return result(EXESS_SUCCESS, sizeof(double));
    }

    break;

  case EXESS_DOUBLE:
    switch (in_datatype) {
    case EXESS_DECIMAL:
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

    break;

  case EXESS_FLOAT:
    switch (in_datatype) {
    case EXESS_DECIMAL:
    case EXESS_DOUBLE:
      if (!(coercions & (ExessCoercionFlags)EXESS_REDUCE_PRECISION)) {
        return result(EXESS_WOULD_REDUCE_PRECISION, 0u);
      }

      *(float*)out = (float)*(const double*)in;
      return result(EXESS_SUCCESS, sizeof(float));

    default:
      if (!(st = coerce_signed(
              &l_out, in_datatype, in, -MAX_FLOAT_INT, MAX_FLOAT_INT))) {
        *(float*)out = (float)l_out;
        return result(EXESS_SUCCESS, sizeof(float));
      }
    }

    break;

  case EXESS_INTEGER:
    if (!(st = coerce_to_long(&l_out, in_datatype, in, coercions))) {
      *(int64_t*)out = l_out;
      return result(EXESS_SUCCESS, sizeof(int64_t));
    }

    break;

  case EXESS_NON_POSITIVE_INTEGER:
    if (!(st = coerce_to_long(&l_out, in_datatype, in, coercions))) {
      if (l_out > 0) {
        return result(EXESS_OUT_OF_RANGE, 0u);
      }

      *(int64_t*)out = l_out;
      return result(EXESS_SUCCESS, sizeof(int64_t));
    }

    break;

  case EXESS_NEGATIVE_INTEGER:
    if (!(st = coerce_to_long(&l_out, in_datatype, in, coercions))) {
      if (l_out >= 0) {
        return result(EXESS_OUT_OF_RANGE, 0u);
      }

      *(int64_t*)out = l_out;
      return result(EXESS_SUCCESS, sizeof(int64_t));
    }

    break;

  case EXESS_LONG:
    if (!(st = coerce_to_long(&l_out, in_datatype, in, coercions))) {
      *(int64_t*)out = l_out;
      return result(EXESS_SUCCESS, sizeof(int64_t));
    }

    break;

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
    if (!(st = coerce_to_ulong(&u_out, in_datatype, in, coercions))) {
      *(uint64_t*)out = u_out;
      return result(EXESS_SUCCESS, sizeof(uint64_t));
    }

    break;

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
    if (!(st = coerce_to_ulong(&u_out, in_datatype, in, coercions))) {
      if (u_out == 0u) {
        return result(EXESS_OUT_OF_RANGE, 0u);
      }

      *(uint64_t*)out = u_out;
      return result(EXESS_SUCCESS, sizeof(uint64_t));
    }

    break;

  case EXESS_DURATION:
  case EXESS_DATETIME:
    break;

  case EXESS_TIME:
    if (in_datatype != EXESS_DATETIME) {
      return result(EXESS_UNSUPPORTED, 0u);
    }

    if ((coercions & (ExessCoercionFlags)EXESS_TRUNCATE)) {
      const ExessDateTime datetime = *(const ExessDateTime*)in;

      const ExessTime time = {{datetime.is_utc ? 0 : EXESS_LOCAL},
                              datetime.hour,
                              datetime.minute,
                              datetime.second,
                              datetime.nanosecond};

      *(ExessTime*)out = time;
      return result(EXESS_SUCCESS, sizeof(ExessTime));
    }

    return result(EXESS_WOULD_TRUNCATE, 0u);

  case EXESS_DATE:
    if (in_datatype != EXESS_DATETIME) {
      return result(EXESS_UNSUPPORTED, 0u);
    }

    if (coercions & (ExessCoercionFlags)EXESS_TRUNCATE) {
      const ExessDateTime datetime = *(const ExessDateTime*)in;

      const ExessDate date = {datetime.year,
                              datetime.month,
                              datetime.day,
                              {datetime.is_utc ? 0 : EXESS_LOCAL}};

      *(ExessDate*)out = date;
      return result(EXESS_SUCCESS, sizeof(ExessDate));
    }

    return result(EXESS_WOULD_TRUNCATE, 0u);

  case EXESS_HEX:
  case EXESS_BASE64:
    break;
  }

  return result(st, 0u);
}

ExessVariant
exess_coerce(const ExessVariant       value,
             const ExessDatatype      datatype,
             const ExessCoercionFlags coercions)
{
  ExessVariant result = value;

  const ExessResult r = exess_coerce_value(coercions,
                                           value.datatype,
                                           exess_value_size(value.datatype),
                                           &value.value,
                                           datatype,
                                           exess_value_size(datatype),
                                           &result.value);

  if (r.status) {
    return exess_make_nothing(r.status);
  }

  result.datatype = datatype;
  return result;
}
