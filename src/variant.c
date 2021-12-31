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
exess_compare_values(const ExessDatatype lhs_datatype,
                     const size_t        lhs_size,
                     const void* const   lhs_value,
                     const ExessDatatype rhs_datatype,
                     const size_t        rhs_size,
                     const void* const   rhs_value)
{
  if (lhs_datatype != rhs_datatype) {
    const char* const lhs_datatype_uri = exess_datatype_uri(lhs_datatype);
    const char* const rhs_datatype_uri = exess_datatype_uri(rhs_datatype);

    return !lhs_datatype_uri   ? -1
           : !rhs_datatype_uri ? 1
                               : strcmp(lhs_datatype_uri, rhs_datatype_uri);
  }

#define EXESS_VALUE_CMP(lhs, rhs) \
  ((lhs) < (rhs)) ? (-1) : ((lhs) > (rhs)) ? 1 : 0

  switch (lhs_datatype) {
  case EXESS_NOTHING:
    break;

  case EXESS_BOOLEAN:
    return EXESS_VALUE_CMP(*(const bool*)lhs_value, *(const bool*)rhs_value);

  case EXESS_DECIMAL:
  case EXESS_DOUBLE:
    return EXESS_VALUE_CMP(*(const double*)lhs_value,
                           *(const double*)rhs_value);

  case EXESS_FLOAT:
    return EXESS_VALUE_CMP(*(const float*)lhs_value, *(const float*)rhs_value);

  case EXESS_INTEGER:
  case EXESS_NON_POSITIVE_INTEGER:
  case EXESS_NEGATIVE_INTEGER:
  case EXESS_LONG:
    return EXESS_VALUE_CMP(*(const int64_t*)lhs_value,
                           *(const int64_t*)rhs_value);

  case EXESS_INT:
    return EXESS_VALUE_CMP(*(const int32_t*)lhs_value,
                           *(const int32_t*)rhs_value);

  case EXESS_SHORT:
    return EXESS_VALUE_CMP(*(const int16_t*)lhs_value,
                           *(const int16_t*)rhs_value);

  case EXESS_BYTE:
    return EXESS_VALUE_CMP(*(const int8_t*)lhs_value,
                           *(const int8_t*)rhs_value);

  case EXESS_NON_NEGATIVE_INTEGER:
  case EXESS_ULONG:
    return EXESS_VALUE_CMP(*(const uint64_t*)lhs_value,
                           *(const uint64_t*)rhs_value);

  case EXESS_UINT:
    return EXESS_VALUE_CMP(*(const uint32_t*)lhs_value,
                           *(const uint32_t*)rhs_value);

  case EXESS_USHORT:
    return EXESS_VALUE_CMP(*(const uint16_t*)lhs_value,
                           *(const uint16_t*)rhs_value);

  case EXESS_UBYTE:
    return EXESS_VALUE_CMP(*(const uint8_t*)lhs_value,
                           *(const uint8_t*)rhs_value);

  case EXESS_POSITIVE_INTEGER:
    return EXESS_VALUE_CMP(*(const uint64_t*)lhs_value,
                           *(const uint64_t*)rhs_value);

  case EXESS_DURATION:
    return exess_duration_compare(*(const ExessDuration*)lhs_value,
                                  *(const ExessDuration*)rhs_value);

  case EXESS_DATETIME:
    return exess_datetime_compare(*(const ExessDateTime*)lhs_value,
                                  *(const ExessDateTime*)rhs_value);

  case EXESS_TIME:
    return exess_time_compare(*(const ExessTime*)lhs_value,
                              *(const ExessTime*)rhs_value);

  case EXESS_DATE:
    return exess_date_compare(*(const ExessDate*)lhs_value,
                              *(const ExessDate*)rhs_value);

  case EXESS_HEX:
  case EXESS_BASE64:
    if (lhs_size != rhs_size) {
      const bool shorter_lhs = lhs_size < rhs_size;
      const int  cmp =
        memcmp(lhs_value, rhs_value, shorter_lhs ? lhs_size : rhs_size);

      return cmp < 0 ? -1 : cmp > 0 ? 1 : shorter_lhs ? -1 : 1;
    }

    return memcmp(lhs_value, rhs_value, lhs_size);
  }

#undef EXESS_VALUE_CMP

  return 0;
}

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

  if (lhs.datatype == EXESS_HEX || lhs.datatype == EXESS_BASE64) {
    return exess_compare_values(lhs.datatype,
                                lhs.value.as_blob.size,
                                (const void*)lhs.value.as_blob.data,
                                rhs.datatype,
                                rhs.value.as_blob.size,
                                (const void*)rhs.value.as_blob.data);
  }

  return exess_compare_values(lhs.datatype,
                              exess_value_size(lhs.datatype),
                              &lhs.value,
                              rhs.datatype,
                              exess_value_size(rhs.datatype),
                              &rhs.value);
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
