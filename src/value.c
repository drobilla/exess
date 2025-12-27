// Copyright 2019-2021 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#include "big_decimal.h"
#include "result.h"
#include "write_utils.h"

#include <exess/exess.h>

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

static inline ExessVariableResult
fixed(const ExessResult result, const size_t write_count)
{
  return VRESULT(result.status, result.count, result.status ? 0U : write_count);
}

static inline ExessVariableResult
integer(const ExessVariableResult result,
        const void* const         value,
        const BigDecimalKind      kind,
        const bool                equals)
{
  const BigDecimal* const decimal = (const BigDecimal*)value;

  return (!result.status && ((decimal->kind == kind) == equals)
            ? VRESULT(EXESS_OUT_OF_RANGE, result.read_count, 0U)
            : result);
}

ExessVariableResult
exess_read_value(const ExessDatatype datatype,
                 const char* const   str,
                 const size_t        out_size,
                 void* const         out)
{
  ExessVariableResult r = {EXESS_UNSUPPORTED, 0, 0};

  if (out_size < exess_value_sizes[datatype]) {
    return VRESULT(EXESS_NO_SPACE, 0U, 0U);
  }

  switch (datatype) {
  case EXESS_NOTHING:
    break;
  case EXESS_BOOLEAN:
    return fixed(exess_read_boolean(str, (bool*)out), sizeof(bool));
  case EXESS_DOUBLE:
    return fixed(exess_read_double(str, (double*)out), sizeof(double));
  case EXESS_FLOAT:
    return fixed(exess_read_float(str, (float*)out), sizeof(float));
  case EXESS_LONG:
    return fixed(exess_read_long(str, (int64_t*)out), sizeof(int64_t));
  case EXESS_INT:
    return fixed(exess_read_int(str, (int32_t*)out), sizeof(int32_t));
  case EXESS_SHORT:
    return fixed(exess_read_short(str, (int16_t*)out), sizeof(int16_t));
  case EXESS_BYTE:
    return fixed(exess_read_byte(str, (int8_t*)out), sizeof(int8_t));
  case EXESS_ULONG:
    return fixed(exess_read_ulong(str, (uint64_t*)out), sizeof(uint64_t));
  case EXESS_UINT:
    return fixed(exess_read_uint(str, (uint32_t*)out), sizeof(uint32_t));
  case EXESS_USHORT:
    return fixed(exess_read_ushort(str, (uint16_t*)out), sizeof(uint16_t));
  case EXESS_UBYTE:
    return fixed(exess_read_ubyte(str, (uint8_t*)out), sizeof(uint8_t));

  case EXESS_DECIMAL:
    return exess_read_decimal(str, out_size, out);
  case EXESS_INTEGER:
    return exess_read_integer(str, out_size, out);

  case EXESS_NON_POSITIVE_INTEGER:
    return integer(
      exess_read_integer(str, out_size, out), out, EXESS_POSITIVE, true);

  case EXESS_NEGATIVE_INTEGER:
    return integer(
      exess_read_integer(str, out_size, out), out, EXESS_NEGATIVE, false);

  case EXESS_NON_NEGATIVE_INTEGER:
    return integer(
      exess_read_integer(str, out_size, out), out, EXESS_NEGATIVE, true);

  case EXESS_POSITIVE_INTEGER:
    return integer(
      exess_read_integer(str, out_size, out), out, EXESS_POSITIVE, false);

  case EXESS_DURATION:
    return fixed(exess_read_duration(str, (ExessDuration*)out),
                 sizeof(ExessDuration));

  case EXESS_YEAR_MONTH_DURATION:
    r = fixed(exess_read_duration(str, (ExessDuration*)out),
              sizeof(ExessDuration));
    return (!r.status && (((const ExessDuration*)out)->seconds ||
                          ((const ExessDuration*)out)->nanoseconds))
             ? VRESULT(EXESS_BAD_VALUE, r.read_count, 0U)
             : r;

  case EXESS_DAY_TIME_DURATION:
    r = fixed(exess_read_duration(str, (ExessDuration*)out),
              sizeof(ExessDuration));
    return (!r.status && ((const ExessDuration*)out)->months)
             ? VRESULT(EXESS_BAD_VALUE, r.read_count, 0U)
             : r;

  case EXESS_DATE_TIME:
    return fixed(exess_read_date_time(str, (ExessDateTime*)out),
                 sizeof(ExessDateTime));

  case EXESS_DATE_TIME_STAMP:
    r = fixed(exess_read_date_time(str, (ExessDateTime*)out),
              sizeof(ExessDateTime));
    return (!r.status &&
            (((const ExessDateTime*)out)->zone == EXESS_TIMEZONE_LOCAL))
             ? VRESULT(EXESS_BAD_VALUE, r.read_count, 0U)
             : r;

  case EXESS_TIME:
    return fixed(exess_read_time(str, (ExessTime*)out), sizeof(ExessTime));
  case EXESS_DATE:
    return fixed(exess_read_date(str, (ExessDate*)out), sizeof(ExessDate));
  case EXESS_HEX:
    return exess_read_hex(str, out_size, out);
  case EXESS_BASE64:
    return exess_read_base64(str, out_size, out);
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
  switch (datatype) {
  case EXESS_NOTHING:
    break;
  case EXESS_BOOLEAN:
    return exess_write_boolean(*(const bool*)value, buf_size, buf);
  case EXESS_DOUBLE:
    return exess_write_double(*(const double*)value, buf_size, buf);
  case EXESS_FLOAT:
    return exess_write_float(*(const float*)value, buf_size, buf);
  case EXESS_LONG:
    return exess_write_long(*(const int64_t*)value, buf_size, buf);
  case EXESS_INT:
    return exess_write_int(*(const int32_t*)value, buf_size, buf);
  case EXESS_SHORT:
    return exess_write_short(*(const int16_t*)value, buf_size, buf);
  case EXESS_BYTE:
    return exess_write_byte(*(const int8_t*)value, buf_size, buf);
  case EXESS_ULONG:
    return exess_write_ulong(*(const uint64_t*)value, buf_size, buf);
  case EXESS_UINT:
    return exess_write_uint(*(const uint32_t*)value, buf_size, buf);
  case EXESS_USHORT:
    return exess_write_ushort(*(const uint16_t*)value, buf_size, buf);
  case EXESS_UBYTE:
    return exess_write_ubyte(*(const uint8_t*)value, buf_size, buf);
  case EXESS_DECIMAL:
    return exess_write_decimal(value_size, value, buf_size, buf);
  case EXESS_INTEGER:
  case EXESS_NON_POSITIVE_INTEGER:
  case EXESS_NEGATIVE_INTEGER:
  case EXESS_NON_NEGATIVE_INTEGER:
  case EXESS_POSITIVE_INTEGER:
    return exess_write_integer(value_size, value, buf_size, buf);
  case EXESS_DURATION:
  case EXESS_YEAR_MONTH_DURATION:
  case EXESS_DAY_TIME_DURATION:
    return exess_write_duration(*(const ExessDuration*)value, buf_size, buf);
  case EXESS_DATE_TIME:
  case EXESS_DATE_TIME_STAMP:
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
