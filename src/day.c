// Copyright 2019-2026 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#include "date_utils.h"
#include "number_utils.h"
#include "read_utils.h"
#include "result.h"
#include "timezone.h"
#include "write_utils.h"

#include <exess/exess.h>

#include <string.h>

EXESS_NONBLOCKING ExessResult
exess_read_day(const char* const str, ExessDay* const out)
{
  memset(out, 0, sizeof(*out));

  // Skip leading absent-year delimiters
  size_t i = skip_whitespace(str);
  if (str[i] != '-') {
    return RESULT(EXESS_EXPECTED_DASH, i);
  }

  // Skip absent-month delimitera
  ++i;
  if (str[i] != '-') {
    return RESULT(EXESS_EXPECTED_DASH, i);
  }

  // Read (month-day delimiter and) day number
  ++i;
  ExessResult r = read_date_d(&out->day, str + i);

  // Read timezone if present
  r.count = i + r.count;
  return read_optional_timezone(&out->zone, str, r);
}

EXESS_NONBLOCKING ExessResult
exess_write_day(const ExessDay value, const size_t buf_size, char* const buf)
{
  if (value.day < 1 || value.day > 31) {
    return end_write(EXESS_BAD_VALUE, buf_size, buf, 0);
  }

  // Write leading absent-year-month delimiters
  ExessResult r = {EXESS_SUCCESS, 0};
  r.count += write_char('-', buf_size, buf, r.count);
  r.count += write_char('-', buf_size, buf, r.count);

  // Write -DD number
  r.count += write_char('-', buf_size, buf, r.count);
  r.count += write_two_digit_number(value.day, buf_size, buf, r.count);

  // Write timezone suffix and end write
  return write_timezone(value.zone, buf_size, buf, r);
}
