/*
  Copyright 2019-2021 David Robillard <d@drobilla.net>

  Permission to use, copy, modify, and/or distribute this software for any
  purpose with or without fee is hereby granted, provided that the above
  copyright notice and this permission notice appear in all copies.

  THIS SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
  WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
  MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
  ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
  WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
  ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
  OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/

#include "exess/exess.h"

#include <math.h>
#include <stdint.h>

/* Limits for the range of integers that can be exactly represented in floating
   point types.  Note that these limits are one less than the largest value,
   since values larger than that may round to it which causes problems with
   perfect round-tripping.  For example, 16777217 when parsed as a float will
   result in 1.6777216E7, which a "lossless" coercion would then convert to
   16777216. */

#define MAX_FLOAT_INT 16777215
#define MAX_DOUBLE_INT 9007199254740991L

static ExessVariant
coerce_long_in_range(const ExessVariant variant,
                     const int64_t      min,
                     const int64_t      max)
{
  const ExessVariant result = exess_coerce(variant, EXESS_LONG, EXESS_LOSSLESS);
  if (result.datatype == EXESS_LONG) {
    if (result.value.as_long < min || result.value.as_long > max) {
      return exess_make_nothing(EXESS_OUT_OF_RANGE);
    }
  }

  return result;
}

static ExessVariant
coerce_to_long(const ExessVariant variant, const ExessCoercionFlags coercions)
{
  switch (variant.datatype) {
  case EXESS_NOTHING:
    return variant;

  case EXESS_BOOLEAN:
    return exess_make_long(variant.value.as_bool);

  case EXESS_DECIMAL:
  case EXESS_DOUBLE:
    if (!(coercions & (ExessCoercionFlags)EXESS_ROUND) &&
        variant.value.as_double > trunc(variant.value.as_double)) {
      return exess_make_nothing(EXESS_WOULD_ROUND);
    }

    if (variant.value.as_double < (double)-MAX_DOUBLE_INT ||
        variant.value.as_double > (double)MAX_DOUBLE_INT) {
      return exess_make_nothing(EXESS_OUT_OF_RANGE);
    }

    return exess_make_long(llrint(variant.value.as_double));

  case EXESS_FLOAT:
    if (!(coercions & (ExessCoercionFlags)EXESS_ROUND) &&
        variant.value.as_float > truncf(variant.value.as_float)) {
      return exess_make_nothing(EXESS_WOULD_ROUND);
    }

    if (variant.value.as_float < (float)-MAX_FLOAT_INT ||
        variant.value.as_float > (float)MAX_FLOAT_INT) {
      return exess_make_nothing(EXESS_OUT_OF_RANGE);
    }

    return exess_make_long(llrintf(variant.value.as_float));

  case EXESS_INTEGER:
  case EXESS_NON_POSITIVE_INTEGER:
  case EXESS_NEGATIVE_INTEGER:
  case EXESS_LONG:
    return exess_make_long(variant.value.as_long);

  case EXESS_INT:
    return exess_make_long(variant.value.as_int);

  case EXESS_SHORT:
    return exess_make_long(variant.value.as_short);

  case EXESS_BYTE:
    return exess_make_long(variant.value.as_byte);

  case EXESS_NON_NEGATIVE_INTEGER:
    return exess_make_long(variant.value.as_long);

  case EXESS_ULONG:
    if (variant.value.as_ulong > INT64_MAX) {
      return exess_make_nothing(EXESS_OUT_OF_RANGE);
    }

    return exess_make_long((int64_t)variant.value.as_ulong);

  case EXESS_UINT:
    return exess_make_long(variant.value.as_uint);

  case EXESS_USHORT:
    return exess_make_long(variant.value.as_ushort);

  case EXESS_UBYTE:
    return exess_make_long(variant.value.as_ubyte);

  case EXESS_POSITIVE_INTEGER:
    if (variant.value.as_ulong > INT64_MAX) {
      return exess_make_nothing(EXESS_OUT_OF_RANGE);
    }

    return exess_make_long((int64_t)variant.value.as_ulong);

  case EXESS_DURATION:
  case EXESS_DATETIME:
  case EXESS_TIME:
  case EXESS_DATE:
  case EXESS_HEX:
  case EXESS_BASE64:
    break;
  }

  return exess_make_nothing(EXESS_UNSUPPORTED);
}

static ExessVariant
coerce_ulong_in_range(const ExessVariant variant, const uint64_t max)
{
  const ExessVariant result =
    exess_coerce(variant, EXESS_ULONG, EXESS_LOSSLESS);

  if (result.datatype == EXESS_ULONG) {
    if (variant.value.as_ulong > max) {
      return exess_make_nothing(EXESS_OUT_OF_RANGE);
    }
  }

  return result;
}

static ExessVariant
coerce_to_ulong(const ExessVariant value, const ExessCoercionFlags coercions)
{
  switch (value.datatype) {
  case EXESS_NOTHING:
    return value;

  case EXESS_BOOLEAN:
    return exess_make_ulong(value.value.as_bool);

  case EXESS_DECIMAL:
  case EXESS_DOUBLE:
    if (!(coercions & (ExessCoercionFlags)EXESS_ROUND) &&
        value.value.as_double > trunc(value.value.as_double)) {
      return exess_make_nothing(EXESS_WOULD_ROUND);
    }

    if (value.value.as_double < 0.0 ||
        value.value.as_double > (double)MAX_DOUBLE_INT) {
      return exess_make_nothing(EXESS_OUT_OF_RANGE);
    }

    return exess_make_ulong((uint64_t)llrint(value.value.as_double));

  case EXESS_FLOAT:
    if (!(coercions & (ExessCoercionFlags)EXESS_ROUND) &&
        value.value.as_float > truncf(value.value.as_float)) {
      return exess_make_nothing(EXESS_WOULD_ROUND);
    }

    if (value.value.as_float < 0.0f ||
        value.value.as_float > (float)MAX_FLOAT_INT) {
      return exess_make_nothing(EXESS_OUT_OF_RANGE);
    }

    return exess_make_ulong((uint64_t)llrintf(value.value.as_float));

  case EXESS_INTEGER:
  case EXESS_NON_POSITIVE_INTEGER:
  case EXESS_NEGATIVE_INTEGER:
  case EXESS_LONG:
    if (value.value.as_long < 0) {
      return exess_make_nothing(EXESS_OUT_OF_RANGE);
    }

    return exess_make_ulong((uint64_t)value.value.as_long);

  case EXESS_INT:
    if (value.value.as_int < 0) {
      return exess_make_nothing(EXESS_OUT_OF_RANGE);
    }

    return exess_make_ulong((uint64_t)value.value.as_int);

  case EXESS_SHORT:
    if (value.value.as_short < 0) {
      return exess_make_nothing(EXESS_OUT_OF_RANGE);
    }

    return exess_make_ulong((uint64_t)value.value.as_short);

  case EXESS_BYTE:
    if (value.value.as_byte < 0) {
      return exess_make_nothing(EXESS_OUT_OF_RANGE);
    }

    return exess_make_ulong((uint64_t)value.value.as_byte);

  case EXESS_NON_NEGATIVE_INTEGER:
  case EXESS_ULONG:
    return exess_make_ulong(value.value.as_ulong);

  case EXESS_UINT:
    return exess_make_ulong(value.value.as_uint);

  case EXESS_USHORT:
    return exess_make_ulong(value.value.as_ushort);

  case EXESS_UBYTE:
    return exess_make_ulong(value.value.as_ubyte);

  case EXESS_POSITIVE_INTEGER:
    return exess_make_ulong(value.value.as_ulong);

  case EXESS_DURATION:
  case EXESS_DATETIME:
  case EXESS_TIME:
  case EXESS_DATE:
  case EXESS_HEX:
  case EXESS_BASE64:
    break;
  }

  return exess_make_nothing(EXESS_UNSUPPORTED);
}

ExessVariant
exess_coerce(const ExessVariant       value,
             const ExessDatatype      datatype,
             const ExessCoercionFlags coercions)
{
  if (datatype == value.datatype) {
    return value;
  }

  ExessVariant result = value;

  switch (datatype) {
  case EXESS_NOTHING:
    break;

  case EXESS_BOOLEAN:
    result = exess_coerce(value, EXESS_LONG, coercions);
    if (result.datatype == EXESS_LONG) {
      if (!(coercions & (ExessCoercionFlags)EXESS_TRUNCATE) &&
          result.value.as_long != 0 && result.value.as_long != 1) {
        return exess_make_nothing(EXESS_WOULD_TRUNCATE);
      }

      return exess_make_boolean(result.value.as_long != 0);
    }
    break;

  case EXESS_DECIMAL:
    if (value.datatype == EXESS_DOUBLE) {
      return exess_make_decimal(value.value.as_double);
    }

    if (value.datatype == EXESS_FLOAT) {
      return exess_make_decimal((double)value.value.as_float);
    }

    result = coerce_long_in_range(value, -MAX_DOUBLE_INT, MAX_DOUBLE_INT);
    if (result.datatype == EXESS_LONG) {
      return exess_make_decimal((double)result.value.as_long);
    }

    break;

  case EXESS_DOUBLE:
    if (value.datatype == EXESS_DECIMAL) {
      return exess_make_double(value.value.as_double);
    }

    if (value.datatype == EXESS_FLOAT) {
      return exess_make_double((double)value.value.as_float);
    }

    result = coerce_long_in_range(value, -MAX_DOUBLE_INT, MAX_DOUBLE_INT);
    if (result.datatype == EXESS_LONG) {
      return exess_make_double((double)result.value.as_long);
    }

    break;

  case EXESS_FLOAT:
    if (value.datatype == EXESS_DECIMAL || value.datatype == EXESS_DOUBLE) {
      if (!(coercions & (ExessCoercionFlags)EXESS_REDUCE_PRECISION)) {
        return exess_make_nothing(EXESS_WOULD_REDUCE_PRECISION);
      }

      return exess_make_float((float)result.value.as_double);
    } else {
      result = coerce_long_in_range(value, -MAX_FLOAT_INT, MAX_FLOAT_INT);
      if (result.datatype == EXESS_LONG) {
        return exess_make_float((float)result.value.as_long);
      }
    }

    break;

  case EXESS_INTEGER:
    result = coerce_to_long(value, coercions);
    break;

  case EXESS_NON_POSITIVE_INTEGER:
    result = coerce_to_long(value, coercions);
    if (result.datatype == EXESS_LONG && result.value.as_long > 0) {
      return exess_make_nothing(EXESS_OUT_OF_RANGE);
    }

    break;

  case EXESS_NEGATIVE_INTEGER:
    result = coerce_to_long(value, coercions);
    if (result.datatype == EXESS_LONG && result.value.as_long >= 0) {
      return exess_make_nothing(EXESS_OUT_OF_RANGE);
    }
    break;

  case EXESS_LONG:
    return coerce_to_long(value, coercions);

  case EXESS_INT:
    result = coerce_long_in_range(value, INT32_MIN, INT32_MAX);
    break;

  case EXESS_SHORT:
    result = coerce_long_in_range(value, INT16_MIN, INT16_MAX);
    break;

  case EXESS_BYTE:
    result = coerce_long_in_range(value, INT8_MIN, INT8_MAX);
    break;

  case EXESS_NON_NEGATIVE_INTEGER:
  case EXESS_ULONG:
    result = coerce_to_ulong(value, coercions);
    break;

  case EXESS_UINT:
    result = coerce_ulong_in_range(value, UINT32_MAX);
    break;

  case EXESS_USHORT:
    result = coerce_ulong_in_range(value, UINT16_MAX);
    break;

  case EXESS_UBYTE:
    result = coerce_ulong_in_range(value, UINT8_MAX);
    break;

  case EXESS_POSITIVE_INTEGER:
    result = coerce_to_ulong(value, coercions);
    if (result.datatype == EXESS_ULONG && result.value.as_ulong == 0u) {
      return exess_make_nothing(EXESS_OUT_OF_RANGE);
    }
    break;

  case EXESS_DURATION:
  case EXESS_DATETIME:
    return exess_make_nothing(EXESS_UNSUPPORTED);

  case EXESS_TIME:
    if (value.datatype != EXESS_DATETIME) {
      return exess_make_nothing(EXESS_UNSUPPORTED);
    }

    if (coercions & (ExessCoercionFlags)EXESS_TRUNCATE) {
      const ExessTime time = {
        {value.value.as_datetime.is_utc ? 0 : EXESS_LOCAL},
        value.value.as_datetime.hour,
        value.value.as_datetime.minute,
        value.value.as_datetime.second,
        value.value.as_datetime.nanosecond};

      return exess_make_time(time);
    }

    return exess_make_nothing(EXESS_WOULD_TRUNCATE);

  case EXESS_DATE:
    if (value.datatype != EXESS_DATETIME) {
      return exess_make_nothing(EXESS_UNSUPPORTED);
    }

    if (coercions & (ExessCoercionFlags)EXESS_TRUNCATE) {
      const ExessDate date = {
        value.value.as_datetime.year,
        value.value.as_datetime.month,
        value.value.as_datetime.day,
        {value.value.as_datetime.is_utc ? 0 : EXESS_LOCAL}};
      return exess_make_date(date);
    }

    return exess_make_nothing(EXESS_WOULD_TRUNCATE);

  case EXESS_HEX:
    return (value.datatype == EXESS_BASE64)
             ? exess_make_hex(value.value.as_blob)
             : exess_make_nothing(EXESS_UNSUPPORTED);

  case EXESS_BASE64:
    return (value.datatype == EXESS_HEX)
             ? exess_make_base64(value.value.as_blob)
             : exess_make_nothing(EXESS_UNSUPPORTED);
  }

  if (result.datatype != EXESS_NOTHING) {
    result.datatype = datatype;
  }

  return result;
}
