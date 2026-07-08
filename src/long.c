// Copyright 2019-2026 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#include "number_utils.h"
#include "read_utils.h"
#include "result.h"
#include "write_utils.h"

#include <exess/exess.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

EXESS_NONBLOCKING EXESS_NONBLOCKING ExessResult
exess_read_long(const char* const str, int64_t* const out)
{
  *out = 0;

  // Skip leading whitespace and read sign if present
  size_t i    = skip_whitespace(str);
  int    sign = 1;
  i += read_sign(&sign, &str[i]);

  // Read digits
  uint64_t    magnitude = 0;
  ExessResult r         = read_digits(&magnitude, str + i);
  if (r.status) {
    return RESULT(r.status, i + r.count);
  }

  i += r.count;

  if (sign > 0) {
    if (magnitude > (uint64_t)INT64_MAX) {
      return RESULT(EXESS_OUT_OF_RANGE, i);
    }

    *out = (int64_t)magnitude;
    return RESULT(EXESS_SUCCESS, i);
  }

  const uint64_t min_magnitude = (uint64_t)(-(INT64_MIN + 1)) + 1;
  if (magnitude > min_magnitude) {
    return RESULT(EXESS_OUT_OF_RANGE, i);
  }

  if (magnitude == min_magnitude) {
    *out = INT64_MIN;
  } else {
    *out = -(int64_t)magnitude;
  }

  return RESULT(EXESS_SUCCESS, i);
}

EXESS_NONBLOCKING EXESS_NONBLOCKING ExessResult
exess_write_long(const int64_t value, const size_t buf_size, char* const buf)
{
  if (value == INT64_MIN) {
    return end_write(
      EXESS_SUCCESS,
      buf_size,
      buf,
      write_string(20, "-9223372036854775808", buf_size, buf, 0));
  }

  const bool     is_negative = value < 0;
  const uint64_t abs_value   = (uint64_t)(is_negative ? -value : value);

  size_t      i = is_negative ? write_char('-', buf_size, buf, 0) : 0;
  ExessResult r = write_digits(abs_value, buf_size, buf, i);

  return end_write(r.status, buf_size, buf, i + r.count);
}
