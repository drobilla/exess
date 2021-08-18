// Copyright 2019-2021 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#include "exess/exess.h"
#include "int_math.h"
#include "read_utils.h"
#include "string_utils.h"
#include "write_utils.h"

#include <stdint.h>
#include <stdlib.h>

ExessResult
exess_read_ulong(uint64_t* const out, const char* const str)
{
  *out = 0;

  // Ensure the first character is a digit
  size_t i = skip_whitespace(str);
  if (!is_digit(str[i])) {
    return result(EXESS_EXPECTED_DIGIT, i);
  }

  // Skip leading zeros
  int n_zeroes = 0;
  while (str[i] == '0') {
    ++i;
    ++n_zeroes;
  }

  // Read digits
  for (; is_digit(str[i]); ++i) {
    const uint64_t next = (*out * 10u) + (uint64_t)(str[i] - '0');
    if (next < *out) {
      *out = 0;
      return result(EXESS_OUT_OF_RANGE, i);
    }

    *out = next;
  }

  return end_read(EXESS_SUCCESS, str, i);
}

ExessResult
write_digits(const uint64_t value,
             const size_t   buf_size,
             char* const    buf,
             const size_t   i)
{
  const uint8_t n_digits = exess_num_digits(value);
  if (!buf) {
    return result(EXESS_SUCCESS, n_digits);
  }

  if (i + n_digits >= buf_size) {
    return end_write(EXESS_NO_SPACE, buf_size, buf, 0);
  }

  // Point s to the end
  char* s = buf + i + n_digits - 1u;

  // Write integer part (right to left)
  uint64_t remaining = value;
  do {
    *s-- = (char)('0' + (remaining % 10));
  } while ((remaining /= 10) > 0);

  return result(EXESS_SUCCESS, n_digits);
}

ExessResult
exess_write_ulong(const uint64_t value, const size_t buf_size, char* const buf)
{
  if (!buf) {
    return result(EXESS_SUCCESS, exess_num_digits(value));
  }

  const ExessResult r = write_digits(value, buf_size, buf, 0);
  return end_write(r.status, buf_size, buf, r.count);
}
