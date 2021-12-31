// Copyright 2019-2021 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#include "write_utils.h"

#include "exess/exess.h"

#include <assert.h>
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
                   const ExessDatatype datatype,
                   const char* const   str)
{
  ExessResult r = {EXESS_UNSUPPORTED, 0};

  out->datatype = datatype;

  if (datatype == EXESS_HEX || datatype == EXESS_BASE64) {
    const ExessVariableResult vr = exess_read_value(
      datatype, out->value.as_blob.size, (void*)out->value.as_blob.data, str);

    r.status                = vr.status;
    r.count                 = vr.read_count;
    out->value.as_blob.size = vr.write_count;

    return r;
  }

  assert(exess_value_size(datatype) <= sizeof(ExessVariant));

  const ExessVariableResult vr =
    exess_read_value(datatype, exess_value_size(datatype), &out->value, str);

  r.status = vr.status;
  r.count  = vr.read_count;
  return r;
}

ExessResult
exess_write_variant(const ExessVariant variant,
                    const size_t       buf_size,
                    char* const        buf)
{
  if (variant.datatype == EXESS_HEX || variant.datatype == EXESS_BASE64) {
    if (!buf_size || !variant.value.as_blob.data) {
      return end_write(EXESS_BAD_VALUE, buf_size, buf, 0);
    }

    return exess_write_value(variant.datatype,
                             variant.value.as_blob.size,
                             (const void*)variant.value.as_blob.data,
                             buf_size,
                             buf);
  }

  return exess_write_value(
    variant.datatype, sizeof(ExessVariant), &variant.value, buf_size, buf);
}
