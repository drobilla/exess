// Copyright 2019-2021 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#include "read_utils.h"
#include "write_utils.h"

#include "exess/exess.h"

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

// Constructors

ExessVariant
exess_make_nothing(const ExessStatus status)
{
  ExessVariant v = {EXESS_NOTHING, .value.as_status = status};
  return v;
}

ExessVariant
exess_make_boolean(const bool value)
{
  ExessVariant v = {EXESS_BOOLEAN, .value.as_bool = value};
  return v;
}

ExessVariant
exess_make_decimal(const double value)
{
  ExessVariant v = {EXESS_DECIMAL, .value.as_double = value};
  return v;
}

ExessVariant
exess_make_double(const double value)
{
  ExessVariant v = {EXESS_DOUBLE, .value.as_double = value};
  return v;
}

ExessVariant
exess_make_float(const float value)
{
  ExessVariant v = {EXESS_FLOAT, .value.as_float = value};
  return v;
}

ExessVariant
exess_make_long(const int64_t value)
{
  ExessVariant v = {EXESS_LONG, .value.as_long = value};
  return v;
}

ExessVariant
exess_make_int(const int32_t value)
{
  ExessVariant v = {EXESS_INT, .value.as_int = value};
  return v;
}

ExessVariant
exess_make_short(const int16_t value)
{
  ExessVariant v = {EXESS_SHORT, .value.as_short = value};
  return v;
}

ExessVariant
exess_make_byte(const int8_t value)
{
  ExessVariant v = {EXESS_BYTE, .value.as_byte = value};
  return v;
}

ExessVariant
exess_make_ulong(const uint64_t value)
{
  ExessVariant v = {EXESS_ULONG, .value.as_ulong = value};
  return v;
}

ExessVariant
exess_make_uint(const uint32_t value)
{
  ExessVariant v = {EXESS_UINT, .value.as_uint = value};
  return v;
}

ExessVariant
exess_make_ushort(const uint16_t value)
{
  ExessVariant v = {EXESS_USHORT, .value.as_ushort = value};
  return v;
}

ExessVariant
exess_make_ubyte(const uint8_t value)
{
  ExessVariant v = {EXESS_UBYTE, .value.as_ubyte = value};
  return v;
}

ExessVariant
exess_make_duration(const ExessDuration value)
{
  ExessVariant v = {EXESS_DURATION, .value.as_duration = value};
  return v;
}

ExessVariant
exess_make_datetime(const ExessDateTime value)
{
  ExessVariant v = {EXESS_DATETIME, .value.as_datetime = value};
  return v;
}

ExessVariant
exess_make_time(const ExessTime value)
{
  ExessVariant v = {EXESS_TIME, .value.as_time = value};
  return v;
}

ExessVariant
exess_make_date(const ExessDate value)
{
  ExessVariant v = {EXESS_DATE, .value.as_date = value};
  return v;
}

ExessVariant
exess_make_hex(const ExessBlob blob)
{
  ExessVariant v = {EXESS_HEX, .value.as_blob = blob};
  return v;
}

ExessVariant
exess_make_base64(const ExessBlob blob)
{
  ExessVariant v = {EXESS_BASE64, .value.as_blob = blob};
  return v;
}

// Accessors

ExessStatus
exess_get_status(const ExessVariant* const variant)
{
  return variant->datatype == EXESS_NOTHING ? variant->value.as_status
                                            : EXESS_SUCCESS;
}
const bool*
exess_get_boolean(const ExessVariant* const variant)
{
  return variant->datatype == EXESS_BOOLEAN ? &variant->value.as_bool : NULL;
}

const double*
exess_get_double(const ExessVariant* const variant)
{
  return (variant->datatype == EXESS_DECIMAL ||
          variant->datatype == EXESS_DOUBLE)
           ? &variant->value.as_double
           : NULL;
}

const float*
exess_get_float(const ExessVariant* const variant)
{
  return variant->datatype == EXESS_FLOAT ? &variant->value.as_float : NULL;
}

const int64_t*
exess_get_long(const ExessVariant* const variant)
{
  return (variant->datatype >= EXESS_INTEGER && variant->datatype <= EXESS_LONG)
           ? &variant->value.as_long
           : NULL;
}

const int32_t*
exess_get_int(const ExessVariant* const variant)
{
  return variant->datatype == EXESS_INT ? &variant->value.as_int : NULL;
}

const int16_t*
exess_get_short(const ExessVariant* const variant)
{
  return variant->datatype == EXESS_SHORT ? &variant->value.as_short : NULL;
}

const int8_t*
exess_get_byte(const ExessVariant* const variant)
{
  return variant->datatype == EXESS_BYTE ? &variant->value.as_byte : NULL;
}

const uint64_t*
exess_get_ulong(const ExessVariant* const variant)
{
  return (variant->datatype == EXESS_NON_NEGATIVE_INTEGER ||
          variant->datatype == EXESS_ULONG ||
          variant->datatype == EXESS_POSITIVE_INTEGER)
           ? &variant->value.as_ulong
           : NULL;
}

const uint32_t*
exess_get_uint(const ExessVariant* const variant)
{
  return variant->datatype == EXESS_UINT ? &variant->value.as_uint : NULL;
}

const uint16_t*
exess_get_ushort(const ExessVariant* const variant)
{
  return variant->datatype == EXESS_USHORT ? &variant->value.as_ushort : NULL;
}

const uint8_t*
exess_get_ubyte(const ExessVariant* const variant)
{
  return variant->datatype == EXESS_UBYTE ? &variant->value.as_ubyte : NULL;
}

const ExessBlob*
exess_get_blob(const ExessVariant* const variant)
{
  return (variant->datatype == EXESS_HEX || variant->datatype == EXESS_BASE64)
           ? &variant->value.as_blob
           : NULL;
}

const ExessDuration*
exess_get_duration(const ExessVariant* const variant)
{
  return variant->datatype == EXESS_DURATION ? &variant->value.as_duration
                                             : NULL;
}

const ExessDateTime*
exess_get_datetime(const ExessVariant* const variant)
{
  return variant->datatype == EXESS_DATETIME ? &variant->value.as_datetime
                                             : NULL;
}

const ExessTime*
exess_get_time(const ExessVariant* const variant)
{
  return variant->datatype == EXESS_TIME ? &variant->value.as_time : NULL;
}

const ExessDate*
exess_get_date(const ExessVariant* const variant)
{
  return variant->datatype == EXESS_DATE ? &variant->value.as_date : NULL;
}

// Comparison

int
exess_compare(const ExessVariant lhs, const ExessVariant rhs)
{
  if (lhs.datatype != rhs.datatype) {
    const char* const lhs_datatype_uri = exess_datatype_uri(lhs.datatype);
    const char* const rhs_datatype_uri = exess_datatype_uri(rhs.datatype);

    return !lhs_datatype_uri   ? -1
           : !rhs_datatype_uri ? 1
                               : strcmp(lhs_datatype_uri, rhs_datatype_uri);
  }

#define EXESS_VALUE_CMP(lhs, rhs) \
  ((lhs) < (rhs)) ? (-1) : ((lhs) > (rhs)) ? 1 : 0

  switch (lhs.datatype) {
  case EXESS_NOTHING:
    break;

  case EXESS_BOOLEAN:
    return EXESS_VALUE_CMP(lhs.value.as_bool, rhs.value.as_bool);

  case EXESS_DECIMAL:
  case EXESS_DOUBLE:
    return EXESS_VALUE_CMP(lhs.value.as_double, rhs.value.as_double);

  case EXESS_FLOAT:
    return EXESS_VALUE_CMP(lhs.value.as_float, rhs.value.as_float);

  case EXESS_INTEGER:
  case EXESS_NON_POSITIVE_INTEGER:
  case EXESS_NEGATIVE_INTEGER:
  case EXESS_LONG:
    return EXESS_VALUE_CMP(lhs.value.as_long, rhs.value.as_long);

  case EXESS_INT:
    return EXESS_VALUE_CMP(lhs.value.as_int, rhs.value.as_int);

  case EXESS_SHORT:
    return EXESS_VALUE_CMP(lhs.value.as_short, rhs.value.as_short);

  case EXESS_BYTE:
    return EXESS_VALUE_CMP(lhs.value.as_byte, rhs.value.as_byte);

  case EXESS_NON_NEGATIVE_INTEGER:
  case EXESS_ULONG:
    return EXESS_VALUE_CMP(lhs.value.as_ulong, rhs.value.as_ulong);

  case EXESS_UINT:
    return EXESS_VALUE_CMP(lhs.value.as_uint, rhs.value.as_uint);

  case EXESS_USHORT:
    return EXESS_VALUE_CMP(lhs.value.as_ushort, rhs.value.as_ushort);

  case EXESS_UBYTE:
    return EXESS_VALUE_CMP(lhs.value.as_ubyte, rhs.value.as_ubyte);

  case EXESS_POSITIVE_INTEGER:
    return EXESS_VALUE_CMP(lhs.value.as_ulong, rhs.value.as_ulong);

  case EXESS_DURATION:
    return exess_duration_compare(lhs.value.as_duration, rhs.value.as_duration);

  case EXESS_DATETIME:
    return exess_datetime_compare(lhs.value.as_datetime, rhs.value.as_datetime);

  case EXESS_TIME:
    return exess_time_compare(lhs.value.as_time, rhs.value.as_time);

  case EXESS_DATE:
    return exess_date_compare(lhs.value.as_date, rhs.value.as_date);

  case EXESS_HEX:
  case EXESS_BASE64:
    if (lhs.value.as_blob.size != rhs.value.as_blob.size) {
      const bool shorter_lhs = lhs.value.as_blob.size < rhs.value.as_blob.size;
      const int  cmp =
        memcmp(lhs.value.as_blob.data,
               rhs.value.as_blob.data,
               shorter_lhs ? lhs.value.as_blob.size : rhs.value.as_blob.size);

      return cmp < 0 ? -1 : cmp > 0 ? 1 : shorter_lhs ? -1 : 1;
    }

    return memcmp(
      lhs.value.as_blob.data, rhs.value.as_blob.data, lhs.value.as_blob.size);
  }

#undef EXESS_VALUE_CMP

  return 0;
}

// Reading and Writing

ExessResult
exess_read_variant(ExessVariant* const out,
                   ExessDatatype       datatype,
                   const char* const   str)
{
  ExessResult r = {EXESS_UNSUPPORTED, 0};

  out->datatype = datatype;

  switch (datatype) {
  case EXESS_NOTHING:
    break;
  case EXESS_BOOLEAN:
    return exess_read_boolean(&out->value.as_bool, str);
  case EXESS_DECIMAL:
    return exess_read_decimal(&out->value.as_double, str);
  case EXESS_DOUBLE:
    return exess_read_double(&out->value.as_double, str);
  case EXESS_FLOAT:
    return exess_read_float(&out->value.as_float, str);
  case EXESS_INTEGER:
    return exess_read_long(&out->value.as_long, str);

  case EXESS_NON_POSITIVE_INTEGER:
    if (!(r = exess_read_long(&out->value.as_long, str)).status) {
      if (out->value.as_long > 0) {
        return result(EXESS_OUT_OF_RANGE, r.count);
      }
    }
    break;

  case EXESS_NEGATIVE_INTEGER:
    if (!(r = exess_read_long(&out->value.as_long, str)).status) {
      if (out->value.as_long >= 0) {
        return result(EXESS_OUT_OF_RANGE, r.count);
      }
    }
    break;

  case EXESS_LONG:
    return exess_read_long(&out->value.as_long, str);
  case EXESS_INT:
    return exess_read_int(&out->value.as_int, str);
  case EXESS_SHORT:
    return exess_read_short(&out->value.as_short, str);
  case EXESS_BYTE:
    return exess_read_byte(&out->value.as_byte, str);
  case EXESS_NON_NEGATIVE_INTEGER:
  case EXESS_ULONG:
    return exess_read_ulong(&out->value.as_ulong, str);
  case EXESS_UINT:
    return exess_read_uint(&out->value.as_uint, str);
  case EXESS_USHORT:
    return exess_read_ushort(&out->value.as_ushort, str);
  case EXESS_UBYTE:
    return exess_read_ubyte(&out->value.as_ubyte, str);

  case EXESS_POSITIVE_INTEGER:
    if (!(r = exess_read_ulong(&out->value.as_ulong, str)).status) {
      if (out->value.as_ulong <= 0) {
        return result(EXESS_OUT_OF_RANGE, r.count);
      }
    }
    break;

  case EXESS_DURATION:
    return exess_read_duration(&out->value.as_duration, str);
  case EXESS_DATETIME:
    return exess_read_datetime(&out->value.as_datetime, str);
  case EXESS_TIME:
    return exess_read_time(&out->value.as_time, str);
  case EXESS_DATE:
    return exess_read_date(&out->value.as_date, str);
  case EXESS_HEX:
    return exess_read_hex(&out->value.as_blob, str);
  case EXESS_BASE64:
    return exess_read_base64(&out->value.as_blob, str);
  }

  return r;
}

ExessResult
exess_write_variant(const ExessVariant variant,
                    const size_t       buf_size,
                    char* const        buf)
{
  if (buf_size > 0) {
    buf[0] = '\0';
  }

  switch (variant.datatype) {
  case EXESS_NOTHING:
    break;
  case EXESS_BOOLEAN:
    return exess_write_boolean(variant.value.as_bool, buf_size, buf);
  case EXESS_DECIMAL:
    return exess_write_decimal(variant.value.as_double, buf_size, buf);
  case EXESS_DOUBLE:
    return exess_write_double(variant.value.as_double, buf_size, buf);
  case EXESS_FLOAT:
    return exess_write_float(variant.value.as_float, buf_size, buf);
  case EXESS_INTEGER:
  case EXESS_NON_POSITIVE_INTEGER:
  case EXESS_NEGATIVE_INTEGER:
  case EXESS_LONG:
    return exess_write_long(variant.value.as_long, buf_size, buf);
  case EXESS_INT:
    return exess_write_int(variant.value.as_int, buf_size, buf);
  case EXESS_SHORT:
    return exess_write_short(variant.value.as_short, buf_size, buf);
  case EXESS_BYTE:
    return exess_write_byte(variant.value.as_byte, buf_size, buf);
  case EXESS_NON_NEGATIVE_INTEGER:
  case EXESS_ULONG:
    return exess_write_ulong(variant.value.as_ulong, buf_size, buf);
  case EXESS_UINT:
    return exess_write_uint(variant.value.as_uint, buf_size, buf);
  case EXESS_USHORT:
    return exess_write_ushort(variant.value.as_ushort, buf_size, buf);
  case EXESS_UBYTE:
    return exess_write_ubyte(variant.value.as_ubyte, buf_size, buf);
  case EXESS_POSITIVE_INTEGER:
    return exess_write_ulong(variant.value.as_ulong, buf_size, buf);
  case EXESS_DURATION:
    return exess_write_duration(variant.value.as_duration, buf_size, buf);
  case EXESS_DATETIME:
    return exess_write_datetime(variant.value.as_datetime, buf_size, buf);
  case EXESS_TIME:
    return exess_write_time(variant.value.as_time, buf_size, buf);
  case EXESS_DATE:
    return exess_write_date(variant.value.as_date, buf_size, buf);
  case EXESS_HEX:
    if (variant.value.as_blob.data) {
      return exess_write_hex(variant.value.as_blob.size,
                             (void*)variant.value.as_blob.data,
                             buf_size,
                             buf);
    }
    break;
  case EXESS_BASE64:
    if (variant.value.as_blob.data) {
      return exess_write_base64(variant.value.as_blob.size,
                                (void*)variant.value.as_blob.data,
                                buf_size,
                                buf);
    }
    break;
  }

  return end_write(EXESS_BAD_VALUE, buf_size, buf, 0);
}
