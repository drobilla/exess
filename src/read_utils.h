// Copyright 2019-2021 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#ifndef EXESS_SRC_READ_UTILS_H
#define EXESS_SRC_READ_UTILS_H

#include "result.h"
#include "string_utils.h"

#include "exess/exess.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

static inline size_t
skip_whitespace(const char* const str)
{
  size_t i = 0;
  while (is_space(str[i])) {
    ++i;
  }

  return i;
}

static inline bool
is_end(const char c)
{
  switch (c) {
  case '\0':
  case ' ':
  case '\f':
  case '\n':
  case '\r':
  case '\t':
  case '\v':
    return true;
  default:
    break;
  }

  return false;
}

ExessResult
read_two_digit_number(uint8_t*    out,
                      uint8_t     min_value,
                      uint8_t     max_value,
                      const char* str);

static inline ExessResult
end_read(const ExessStatus status, const char* str, const size_t i)
{
  return result((status || is_end(str[i])) ? status : EXESS_EXPECTED_END, i);
}

#endif // EXESS_SRC_READ_UTILS_H
