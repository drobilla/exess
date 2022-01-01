// Copyright 2019-2021 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#include "exess/exess.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

int
exess_value_compare(const ExessDatatype lhs_datatype,
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
