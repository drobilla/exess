// Copyright 2019-2025 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#include <exess/exess.h>

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

static ExessOrder
compare_datatypes(const ExessDatatype lhs_datatype,
                  const ExessDatatype rhs_datatype)
{
  const char* const lhs_name = exess_datatype_name(lhs_datatype);
  const char* const rhs_name = exess_datatype_name(rhs_datatype);
  return !lhs_name                          ? EXESS_ORDER_MAYBE_LESS
         : !rhs_name                        ? EXESS_ORDER_MAYBE_GREATER
         : (strcmp(lhs_name, rhs_name) < 0) ? EXESS_ORDER_MAYBE_LESS
                                            : EXESS_ORDER_MAYBE_GREATER;
}

static ExessOrder
compare_blob(const size_t      lhs_size,
             const void* const lhs_value,
             const size_t      rhs_size,
             const void* const rhs_value)
{
  const bool   shorter_lhs = lhs_size < rhs_size;
  const size_t cmp_size    = shorter_lhs ? lhs_size : rhs_size;
  const int    cmp         = memcmp(lhs_value, rhs_value, cmp_size);

  return cmp < 0                  ? EXESS_ORDER_MAYBE_LESS
         : cmp > 0                ? EXESS_ORDER_MAYBE_GREATER
         : shorter_lhs            ? EXESS_ORDER_MAYBE_LESS
         : (lhs_size == rhs_size) ? EXESS_ORDER_EQUAL
                                  : EXESS_ORDER_MAYBE_GREATER;
}

ExessOrder
exess_compare_value(const ExessDatatype lhs_datatype,
                    const size_t        lhs_size,
                    const void* const   lhs_value,
                    const ExessDatatype rhs_datatype,
                    const size_t        rhs_size,
                    const void* const   rhs_value)
{
  if (lhs_datatype != rhs_datatype) {
    return compare_datatypes(lhs_datatype, rhs_datatype);
  }

#define COMPARE(lhs, rhs)                          \
  ((lhs) < (rhs))   ? EXESS_ORDER_STRICTLY_LESS    \
  : ((rhs) < (lhs)) ? EXESS_ORDER_STRICTLY_GREATER \
                    : EXESS_ORDER_EQUAL

  switch (lhs_datatype) {
  case EXESS_NOTHING:
    break;
  case EXESS_BOOLEAN:
    return COMPARE(*(const bool*)lhs_value, *(const bool*)rhs_value);
  case EXESS_DECIMAL:
  case EXESS_DOUBLE:
    return COMPARE(*(const double*)lhs_value, *(const double*)rhs_value);
  case EXESS_FLOAT:
    return COMPARE(*(const float*)lhs_value, *(const float*)rhs_value);
  case EXESS_INTEGER:
  case EXESS_NON_POSITIVE_INTEGER:
  case EXESS_NEGATIVE_INTEGER:
  case EXESS_LONG:
    return COMPARE(*(const int64_t*)lhs_value, *(const int64_t*)rhs_value);
  case EXESS_INT:
    return COMPARE(*(const int32_t*)lhs_value, *(const int32_t*)rhs_value);
  case EXESS_SHORT:
    return COMPARE(*(const int16_t*)lhs_value, *(const int16_t*)rhs_value);
  case EXESS_BYTE:
    return COMPARE(*(const int8_t*)lhs_value, *(const int8_t*)rhs_value);
  case EXESS_NON_NEGATIVE_INTEGER:
  case EXESS_ULONG:
    return COMPARE(*(const uint64_t*)lhs_value, *(const uint64_t*)rhs_value);
  case EXESS_UINT:
    return COMPARE(*(const uint32_t*)lhs_value, *(const uint32_t*)rhs_value);
  case EXESS_USHORT:
    return COMPARE(*(const uint16_t*)lhs_value, *(const uint16_t*)rhs_value);
  case EXESS_UBYTE:
    return COMPARE(*(const uint8_t*)lhs_value, *(const uint8_t*)rhs_value);
  case EXESS_POSITIVE_INTEGER:
    return COMPARE(*(const uint64_t*)lhs_value, *(const uint64_t*)rhs_value);
  case EXESS_DURATION:
    return exess_compare_duration(*(const ExessDuration*)lhs_value,
                                  *(const ExessDuration*)rhs_value);

  case EXESS_DATE_TIME:
    return exess_compare_date_time(*(const ExessDateTime*)lhs_value,
                                   *(const ExessDateTime*)rhs_value);

  case EXESS_TIME:
    return exess_compare_time(*(const ExessTime*)lhs_value,
                              *(const ExessTime*)rhs_value);

  case EXESS_DATE:
    return exess_compare_date(*(const ExessDate*)lhs_value,
                              *(const ExessDate*)rhs_value);

  case EXESS_HEX:
  case EXESS_BASE64:
    return compare_blob(lhs_size, lhs_value, rhs_size, rhs_value);
  }

#undef COMPARE

  return EXESS_ORDER_EQUAL;
}
