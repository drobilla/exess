// Copyright 2019-2021 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#include "result.h"
#include "write_utils.h"

#include <exess/exess.h>

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

static inline ExessVariableResult
fixed(const ExessResult result, const size_t write_count)
{
  return vresult(result.status, result.count, result.status ? 0U : write_count);
}

ExessVariableResult
exess_read_value(const ExessDatatype datatype,
                 const size_t        out_size,
                 void* const         out,
                 const char* const   str)
{
  ExessVariableResult r = {EXESS_UNSUPPORTED, 0, 0};

  if (out_size < exess_value_size(datatype)) {
    r.status = EXESS_NO_SPACE;
    return r;
  }

  switch (datatype) {
  case EXESS_NOTHING:
    break;
  case EXESS_BOOLEAN:
    return fixed(exess_read_boolean((bool*)out, str), sizeof(bool));
  case EXESS_DECIMAL:
    return fixed(exess_read_decimal((double*)out, str), sizeof(double));
  case EXESS_DOUBLE:
    return fixed(exess_read_double((double*)out, str), sizeof(double));
  case EXESS_FLOAT:
    return fixed(exess_read_float((float*)out, str), sizeof(float));
  case EXESS_INTEGER:
    return fixed(exess_read_long((int64_t*)out, str), sizeof(int64_t));

  case EXESS_NON_POSITIVE_INTEGER:
    r = fixed(exess_read_long((int64_t*)out, str), sizeof(int64_t));
    return (!r.status && *(const int64_t*)out > 0)
             ? vresult(EXESS_OUT_OF_RANGE, r.read_count, 0U)
             : r;

  case EXESS_NEGATIVE_INTEGER:
    r = fixed(exess_read_long((int64_t*)out, str), sizeof(int64_t));
    return (!r.status && *(const int64_t*)out >= 0)
             ? vresult(EXESS_OUT_OF_RANGE, r.read_count, 0U)
             : r;

  case EXESS_LONG:
    return fixed(exess_read_long((int64_t*)out, str), sizeof(int64_t));
  case EXESS_INT:
    return fixed(exess_read_int((int32_t*)out, str), sizeof(int32_t));
  case EXESS_SHORT:
    return fixed(exess_read_short((int16_t*)out, str), sizeof(int16_t));
  case EXESS_BYTE:
    return fixed(exess_read_byte((int8_t*)out, str), sizeof(int8_t));
  case EXESS_NON_NEGATIVE_INTEGER:
  case EXESS_ULONG:
    return fixed(exess_read_ulong((uint64_t*)out, str), sizeof(uint64_t));
  case EXESS_UINT:
    return fixed(exess_read_uint((uint32_t*)out, str), sizeof(uint32_t));
  case EXESS_USHORT:
    return fixed(exess_read_ushort((uint16_t*)out, str), sizeof(uint16_t));
  case EXESS_UBYTE:
    return fixed(exess_read_ubyte((uint8_t*)out, str), sizeof(uint8_t));

  case EXESS_POSITIVE_INTEGER:
    r = fixed(exess_read_ulong((uint64_t*)out, str), sizeof(uint64_t));
    return (!r.status && *(const uint64_t*)out == 0)
             ? vresult(EXESS_OUT_OF_RANGE, r.read_count, 0U)
             : r;

  case EXESS_DURATION:
    return fixed(exess_read_duration((ExessDuration*)out, str),
                 sizeof(ExessDuration));

  case EXESS_DATE_TIME:
    return fixed(exess_read_date_time((ExessDateTime*)out, str),
                 sizeof(ExessDateTime));

  case EXESS_TIME:
    return fixed(exess_read_time((ExessTime*)out, str), sizeof(ExessTime));
  case EXESS_DATE:
    return fixed(exess_read_date((ExessDate*)out, str), sizeof(ExessDate));
  case EXESS_HEX:
    return exess_read_hex(out_size, out, str);
  case EXESS_BASE64:
    return exess_read_base64(out_size, out, str);
  }

  return r;
}

ExessResult
exess_write_value(const ExessDatatype datatype,
                  const size_t        value_size,
                  const void* const   value,
                  const size_t        buf_size,
                  char* const         buf)
{
  if (buf && buf_size > 0) {
    buf[0] = '\0';
  }

  switch (datatype) {
  case EXESS_NOTHING:
    break;
  case EXESS_BOOLEAN:
    return exess_write_boolean(*(const bool*)value, buf_size, buf);
  case EXESS_DECIMAL:
    return exess_write_decimal(*(const double*)value, buf_size, buf);
  case EXESS_DOUBLE:
    return exess_write_double(*(const double*)value, buf_size, buf);
  case EXESS_FLOAT:
    return exess_write_float(*(const float*)value, buf_size, buf);
  case EXESS_INTEGER:
  case EXESS_NON_POSITIVE_INTEGER:
  case EXESS_NEGATIVE_INTEGER:
  case EXESS_LONG:
    return exess_write_long(*(const int64_t*)value, buf_size, buf);
  case EXESS_INT:
    return exess_write_int(*(const int32_t*)value, buf_size, buf);
  case EXESS_SHORT:
    return exess_write_short(*(const int16_t*)value, buf_size, buf);
  case EXESS_BYTE:
    return exess_write_byte(*(const int8_t*)value, buf_size, buf);
  case EXESS_NON_NEGATIVE_INTEGER:
  case EXESS_ULONG:
    return exess_write_ulong(*(const uint64_t*)value, buf_size, buf);
  case EXESS_UINT:
    return exess_write_uint(*(const uint32_t*)value, buf_size, buf);
  case EXESS_USHORT:
    return exess_write_ushort(*(const uint16_t*)value, buf_size, buf);
  case EXESS_UBYTE:
    return exess_write_ubyte(*(const uint8_t*)value, buf_size, buf);
  case EXESS_POSITIVE_INTEGER:
    return exess_write_ulong(*(const uint64_t*)value, buf_size, buf);
  case EXESS_DURATION:
    return exess_write_duration(*(const ExessDuration*)value, buf_size, buf);
  case EXESS_DATE_TIME:
    return exess_write_date_time(*(const ExessDateTime*)value, buf_size, buf);
  case EXESS_TIME:
    return exess_write_time(*(const ExessTime*)value, buf_size, buf);
  case EXESS_DATE:
    return exess_write_date(*(const ExessDate*)value, buf_size, buf);
  case EXESS_HEX:
    return exess_write_hex(value_size, value, buf_size, buf);
  case EXESS_BASE64:
    return exess_write_base64(value_size, value, buf_size, buf);
  }

  return end_write(EXESS_BAD_VALUE, buf_size, buf, 0);
}
