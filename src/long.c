// Copyright 2019-2021 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#include "int_math.h"
#include "read_utils.h"
#include "result.h"
#include "write_utils.h"

#include "exess/exess.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

ExessResult
exess_read_long(int64_t* const out, const char* const str)
{
  *out = 0;

  // Read leading sign if present
  size_t i    = skip_whitespace(str);
  int    sign = 1;
  if (str[i] == '-') {
    sign = -1;
    ++i;
  } else if (str[i] == '+') {
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
    if (magnitude > (uint64_t)INT64_MAX) {
      return result(EXESS_OUT_OF_RANGE, i);
    }

    *out = (int64_t)magnitude;
    return end_read(EXESS_SUCCESS, str, i);
  }

  const uint64_t min_magnitude = (uint64_t)(-(INT64_MIN + 1)) + 1;
  if (magnitude > min_magnitude) {
    return result(EXESS_OUT_OF_RANGE, i);
  }

  if (magnitude == min_magnitude) {
    *out = INT64_MIN;
  } else {
    *out = -(int64_t)magnitude;
  }

  return end_read(r.status, str, i);
}

static size_t
exess_long_string_length(const int64_t value)
{
  if (value == INT64_MIN) {
    return 20;
  }

  if (value < 0) {
    return 1U + exess_num_digits((uint64_t)-value);
  }

  return exess_num_digits((uint64_t)value);
}

ExessResult
exess_write_long(const int64_t value, const size_t buf_size, char* const buf)
{
  if (!buf) {
    return result(EXESS_SUCCESS, exess_long_string_length(value));
  }

  if (value == INT64_MIN) {
    return end_write(
      EXESS_SUCCESS,
      buf_size,
      buf,
      write_string(20, "-9223372036854775808", buf_size, buf, 0));
  }

  const bool     is_negative = value < 0;
  const uint64_t abs_value   = (uint64_t)(is_negative ? -value : value);

  size_t      i = (is_negative) ? write_char('-', buf_size, buf, 0) : 0;
  ExessResult r = write_digits(abs_value, buf_size, buf, i);

  return end_write(r.status, buf_size, buf, i + r.count);
}
