// Copyright 2019-2024 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#include "read_utils.h"

#include "result.h"
#include "string_utils.h"

#include <exess/exess.h>

size_t
skip_whitespace(const char* const str)
{
  size_t i = 0;
  while (is_space(str[i])) {
    ++i;
  }

  return i;
}

bool
is_end(const char c)
{
  return c == '\0' || c == ' ' || c == '\f' || c == '\n' || c == '\r' ||
         c == '\t' || c == '\v';
}

ExessResult
read_two_digit_number(uint8_t* const    out,
                      const uint8_t     min_value,
                      const uint8_t     max_value,
                      const char* const str)
{
  // Read digits
  size_t i = 0;
  for (; i < 2 && is_digit(str[i]); ++i) {
    *out = (uint8_t)((*out * 10) + (str[i] - '0'));
  }

  // Ensure there are exactly the expected number of digits
  if (i != 2) {
    return result(EXESS_EXPECTED_DIGIT, i);
  }

  // Ensure value is in range
  if (*out < min_value || *out > max_value) {
    return result(EXESS_OUT_OF_RANGE, i);
  }

  return result(EXESS_SUCCESS, i);
}

size_t
read_sign(int* const sign, const char* const str)
{
  if (*str == '-') {
    *sign = -1;
    return 1U;
  }

  *sign = 1;
  return (*str == '+') ? 1U : 0U;
}

ExessResult
end_read(const ExessStatus status, const char* str, const size_t i)
{
  return result((status || is_end(str[i])) ? status : EXESS_EXPECTED_END, i);
}
