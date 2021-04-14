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

#include "timezone.h"
#include "read_utils.h"
#include "string_utils.h"
#include "write_utils.h"

#include "exess/exess.h"

#include <stdint.h>
#include <stdlib.h>

ExessResult
exess_read_timezone(ExessTimezone* const out, const char* const str)
{
  out->quarter_hours = INT8_MAX;

  // Start at the beginning (no whitespace skipping here)
  size_t i = 0;

  // Handle UTC special case
  if (str[i] == 'Z') {
    out->quarter_hours = 0;
    return result(EXESS_SUCCESS, i + 1);
  }

  // Read leading sign (required)
  int sign = 1;
  switch (str[i]) {
  case '+':
    ++i;
    break;
  case '-':
    sign = -1;
    ++i;
    break;
  default:
    return result(EXESS_EXPECTED_SIGN, i);
  }

  const char h0 = str[i];
  if (!is_digit(h0)) {
    return result(EXESS_EXPECTED_DIGIT, i);
  }

  const char h1 = str[++i];
  if (!is_digit(h1)) {
    return result(EXESS_EXPECTED_DIGIT, i);
  }

  ++i;

  const int8_t hour = (int8_t)(sign * (10 * (h0 - '0') + (h1 - '0')));
  if (hour > 14 || hour < -14) {
    return result(EXESS_OUT_OF_RANGE, i);
  }

  if (str[i] != ':') {
    return result(EXESS_EXPECTED_COLON, i);
  }

  const char m0 = str[++i];
  if (!is_digit(m0)) {
    return result(EXESS_EXPECTED_DIGIT, i);
  }

  const char m1 = str[++i];
  if (!is_digit(m1)) {
    return result(EXESS_EXPECTED_DIGIT, i);
  }

  const int8_t minute = (int8_t)(sign * (10 * (m0 - '0') + (m1 - '0')));

  ++i;

  if (minute % 15) {
    return result(EXESS_UNSUPPORTED, i);
  }

  if (minute > 59 || minute < -59) {
    return result(EXESS_OUT_OF_RANGE, i);
  }

  const int8_t quarters = (int8_t)(4 * hour + minute / 15);
  if (quarters < -56 || quarters > 56) {
    return result(EXESS_OUT_OF_RANGE, i);
  }

  out->quarter_hours = quarters;

  return result(EXESS_SUCCESS, i);
}

ExessResult
write_timezone(const ExessTimezone value,
               const size_t        buf_size,
               char* const         buf,
               size_t              o)
{
  if (value.quarter_hours == EXESS_LOCAL) {
    return result(EXESS_SUCCESS, 0);
  }

  if (value.quarter_hours < -56 || value.quarter_hours > 56) {
    return result(EXESS_BAD_VALUE, 0);
  }

  if (!buf) {
    return result(EXESS_SUCCESS, value.quarter_hours == 0 ? 1 : 6);
  }

  if (value.quarter_hours == 0) {
    write_char('Z', buf_size, buf, o);
    return result(EXESS_SUCCESS, 1);
  }

  const uint8_t abs_quarters = (uint8_t)abs(value.quarter_hours);
  const uint8_t abs_hour     = abs_quarters / 4;
  const uint8_t abs_minute   = (uint8_t)(15u * (abs_quarters % 4u));

  size_t n = 0;
  n += write_char(value.quarter_hours < 0 ? '-' : '+', buf_size, buf, o + n);
  n += write_two_digit_number(abs_hour, buf_size, buf, o + n);
  n += write_char(':', buf_size, buf, o + n);
  n += write_two_digit_number(abs_minute, buf_size, buf, o + n);

  return result(EXESS_SUCCESS, n);
}
