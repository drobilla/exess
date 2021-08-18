// Copyright 2019-2021 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#include "write_utils.h"
#include "read_utils.h"

#include "exess/exess.h"

#include <string.h>

size_t
write_two_digit_number(const uint8_t value,
                       const size_t  buf_size,
                       char* const   buf,
                       const size_t  i)
{
  if (buf_size >= i + 1) {
    buf[i]     = (char)((value >= 10) ? ('0' + value / 10) : '0');
    buf[i + 1] = (char)('0' + (value % 10));
  }

  return 2;
}

ExessResult
write_special(const size_t      string_length,
              const char* const string,
              const size_t      buf_size,
              char* const       buf)
{
  if (buf_size < string_length + 1) {
    return end_write(EXESS_NO_SPACE, buf_size, buf, 0);
  }

  memcpy(buf, string, string_length + 1);
  return result(EXESS_SUCCESS, string_length);
}
