// Copyright 2019-2025 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#include "number_utils.h"

#include "char_utils.h"
#include "int_math.h"
#include "result.h"

#include <exess/exess.h>

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
    return RESULT(EXESS_EXPECTED_DIGIT, i);
  }

  // Ensure value is in range
  if (*out < min_value || *out > max_value) {
    return RESULT(EXESS_OUT_OF_RANGE, i);
  }

  return RESULT(EXESS_SUCCESS, i);
}

size_t
write_two_digit_number(const uint8_t value,
                       const size_t  buf_size,
                       char* const   buf,
                       const size_t  i)
{
  if (i + 1 < buf_size) {
    buf[i]     = (char)((value >= 10) ? ('0' + (value / 10)) : '0');
    buf[i + 1] = (char)('0' + (value % 10));
  }

  return 2;
}

ExessResult
read_digits(uint64_t* const out, const char* const str)
{
  // Ensure the first character is a digit
  size_t i = 0;
  if (!is_digit(str[i])) {
    return RESULT(EXESS_EXPECTED_DIGIT, i);
  }

  // Skip leading zeros
  while (str[i] == '0') {
    ++i;
  }

  // Read digits
  for (; is_digit(str[i]); ++i) {
    const uint64_t next = (*out * 10U) + (uint64_t)(str[i] - '0');
    if (next < *out) {
      *out = 0;
      return RESULT(EXESS_OUT_OF_RANGE, i);
    }

    *out = next;
  }

  return RESULT(EXESS_SUCCESS, i);
}

ExessResult
write_digits(const uint64_t value,
             const size_t   buf_size,
             char* const    buf,
             const size_t   i)
{
  const uint8_t n_digits = exess_num_digits(value);
  if (i + n_digits >= buf_size) {
    return RESULT(EXESS_NO_SPACE, n_digits);
  }

  // Point s to the end
  char* s = buf + i + n_digits - 1U;

  // Write integer part (right to left)
  uint64_t remaining = value;
  do {
    *s-- = (char)('0' + (remaining % 10));
  } while ((remaining /= 10) > 0);

  return RESULT(EXESS_SUCCESS, n_digits);
}
