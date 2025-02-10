// Copyright 2019-2024 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#include "timezone.h"
#include "read_utils.h"
#include "result.h"
#include "write_utils.h"

#include <exess/exess.h>

#include <stdint.h>
#include <stdlib.h>

ExessTimezone
exess_timezone(const int8_t hours, const int8_t minutes)
{
  if (hours < -14 || hours > 14) {
    return EXESS_LOCAL;
  }

  if (hours < 0 && minutes != 0 && minutes != -15 && minutes != -30 &&
      minutes != -45) {
    return EXESS_LOCAL;
  }

  if (hours > 0 && minutes != 0 && minutes != 15 && minutes != 30 &&
      minutes != 45) {
    return EXESS_LOCAL;
  }

  return (int8_t)((4 * hours) + (minutes / 15));
}

ExessResult
read_timezone(ExessTimezone* const out, const char* const str)
{
  *out = EXESS_LOCAL;

  // Handle UTC special case
  size_t i = 0;
  if (str[i] == 'Z') {
    *out = 0;
    return result(EXESS_SUCCESS, i + 1);
  }

  // Read leading sign (required)
  int sign = 1;
  if (str[i] == '-') {
    sign = -1;
  } else if (str[i] != '+') {
    return result(EXESS_EXPECTED_SIGN, i);
  }

  ++i;

  // Read hour digits
  uint8_t     hh = 0U;
  ExessResult r  = read_two_digit_number(&hh, 0U, 14U, str + i);
  if (r.status) {
    return result(r.status, i + r.count);
  }

  i += 2U;

  // Check colon
  if (str[i] != ':') {
    return result(EXESS_EXPECTED_COLON, i);
  }

  ++i;

  // Read minute digits
  uint8_t mm = 0U;
  r          = read_two_digit_number(&mm, 0U, 59U, str + i);
  if (r.status) {
    return result(r.status, i + r.count);
  }

  i += 2U;

  // Calculate signed hour and minute
  const int8_t hour   = (int8_t)(sign * hh);
  const int8_t minute = (int8_t)(sign * mm);
  if (minute % 15) {
    return result(EXESS_UNSUPPORTED, i);
  }

  // Convert to quarter hours
  const int8_t quarters = (int8_t)((4 * hour) + (minute / 15));
  if (quarters < -56 || quarters > 56) {
    return result(EXESS_OUT_OF_RANGE, i);
  }

  *out = quarters;
  return result(EXESS_SUCCESS, i);
}

ExessResult
write_timezone(const ExessTimezone value,
               const size_t        buf_size,
               char* const         buf,
               size_t              o)
{
  if (value == EXESS_LOCAL) {
    return result(EXESS_SUCCESS, 0);
  }

  if (value < -56 || value > 56) {
    return result(EXESS_BAD_VALUE, 0);
  }

  if (!buf) {
    return result(EXESS_SUCCESS, value == 0 ? 1 : 6);
  }

  if (value == 0) {
    write_char('Z', buf_size, buf, o);
    return result(EXESS_SUCCESS, 1);
  }

  const uint8_t abs_quarters = (uint8_t)abs(value);
  const uint8_t abs_hour     = abs_quarters / 4;
  const uint8_t abs_minute   = (uint8_t)(15U * (abs_quarters % 4U));

  size_t n = 0;
  n += write_char(value < 0 ? '-' : '+', buf_size, buf, o + n);
  n += write_two_digit_number(abs_hour, buf_size, buf, o + n);
  n += write_char(':', buf_size, buf, o + n);
  n += write_two_digit_number(abs_minute, buf_size, buf, o + n);

  return result(EXESS_SUCCESS, n);
}
