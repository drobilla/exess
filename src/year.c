// Copyright 2019-2021 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#include "date_utils.h"
#include "int_math.h"
#include "macros.h"
#include "result.h"
#include "write_utils.h"

#include "exess/exess.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

ExessResult
read_year_number(int16_t* const out, const char* const str)
{
  *out = 0;

  // Read leading sign if present
  size_t i    = 0;
  int    sign = 1;
  if (str[i] == '-') {
    sign = -1;
    ++i;
  }

  // Read digits
  uint64_t    magnitude = 0;
  ExessResult r         = exess_read_ulong(&magnitude, str + i);
  if (r.status > EXESS_EXPECTED_END) {
    return result(r.status, i + r.count);
  }

  i += r.count;

  if (sign > 0) {
    if (magnitude > (uint16_t)INT16_MAX) {
      return result(EXESS_OUT_OF_RANGE, i);
    }

    *out = (int16_t)magnitude;
  } else {
    const uint16_t min_magnitude = (uint16_t)(-(INT16_MIN + 1)) + 1;
    if (magnitude > min_magnitude) {
      return result(EXESS_OUT_OF_RANGE, i);
    }

    if (magnitude == min_magnitude) {
      *out = INT16_MIN;
    } else {
      *out = (int16_t) - (int16_t)magnitude;
    }
  }

  return result(r.count >= 4 ? EXESS_SUCCESS : EXESS_EXPECTED_DIGIT, i);
}

ExessResult
write_year_number(const int16_t value, const size_t buf_size, char* const buf)
{
  const uint32_t abs_year    = (uint32_t)abs(value);
  const uint8_t  n_digits    = exess_num_digits(abs_year);
  const bool     is_negative = value < 0;

  if (!buf) {
    return result(EXESS_SUCCESS, is_negative + MAX(4U, n_digits));
  }

  // Write sign
  size_t i = 0;
  if (is_negative) {
    i += write_char('-', buf_size, buf, i);
  }

  // Write leading zeros to ensure we have at least 4 year digits
  for (size_t j = n_digits; j < 4; ++j) {
    i += write_char('0', buf_size, buf, i);
  }

  const ExessResult yr = exess_write_uint(abs_year, buf_size - i, buf + i);

  return end_write(yr.status, buf_size, buf, i + yr.count);
}
