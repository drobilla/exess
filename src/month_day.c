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
exess_read_month_day(const char* const str, ExessMonthDay* const out)
{
  memset(out, 0, sizeof(*out));

  // Skip leading absent-year delimiter
  size_t i = skip_whitespace(str);
  if (str[i] != '-') {
    return RESULT(EXESS_EXPECTED_DASH, i);
  }

  // Read (leading month delimiter and) month
  ++i;
  ExessResult r = read_date_m(&out->month, str + i);
  i += r.count;

  if (!r.status) {
    // Read (month-day delimiter and) day
    r = read_date_d(&out->day, str + i);
  }

  // Read timezone if present
  r.count = i + r.count;
  return read_optional_timezone(&out->zone, str, r);
}

EXESS_NONBLOCKING ExessResult
exess_write_month_day(const ExessMonthDay value,
                      const size_t        buf_size,
                      char* const         buf)
{
  if (value.month < 1 || value.month > 12 || value.day < 1 || value.day > 31) {
    return end_write(EXESS_BAD_VALUE, buf_size, buf, 0);
  }

  // Write --MM-DD numbers
  ExessResult r = {EXESS_SUCCESS, 0};
  r.count += write_char('-', buf_size, buf, r.count);
  r.count += write_char('-', buf_size, buf, r.count);
  r.count += write_two_digit_number(value.month, buf_size, buf, r.count);
  r.count += write_char('-', buf_size, buf, r.count);
  r.count += write_two_digit_number(value.day, buf_size, buf, r.count);

  // Write timezone suffix and end write
  return write_timezone(value.zone, buf_size, buf, r);
}
