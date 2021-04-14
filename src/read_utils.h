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

#ifndef EXESS_READ_UTILS_H
#define EXESS_READ_UTILS_H

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

static inline ExessResult
result(const ExessStatus status, const size_t count)
{
  const ExessResult r = {status, count};
  return r;
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

#endif // EXESS_READ_UTILS_H
