// Copyright 2019-2021 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#include "date_utils.h"
#include "read_utils.h"
#include "result.h"
#include "timezone.h"
#include "write_utils.h"

#include "exess/exess.h"

#include <stdlib.h>
#include <string.h>

int
exess_date_compare(const ExessDate lhs, const ExessDate rhs)
{
  ExessDateTime lhs_datetime = {
    lhs.year, lhs.month, lhs.day, lhs.zone != EXESS_LOCAL, 0, 0, 0, 0};

  ExessDateTime rhs_datetime = {
    rhs.year, rhs.month, rhs.day, rhs.zone != EXESS_LOCAL, 0, 0, 0, 0};

  if (lhs.zone != EXESS_LOCAL) {
    const ExessDuration lhs_tz_duration = {0u, -lhs.zone * 15 * 60, 0};

    lhs_datetime = exess_add_datetime_duration(lhs_datetime, lhs_tz_duration);
  }

  if (rhs.zone != EXESS_LOCAL) {
    const ExessDuration rhs_tz_duration = {0u, -rhs.zone * 15 * 60, 0};

    rhs_datetime = exess_add_datetime_duration(rhs_datetime, rhs_tz_duration);
  }

  return exess_datetime_compare(lhs_datetime, rhs_datetime);
}

ExessResult
read_date_numbers(ExessDate* const out, const char* const str)
{
  // Read year at the beginning
  size_t      i = skip_whitespace(str);
  ExessResult r = read_year_number(&out->year, str + i);
  if (r.status) {
    return result(r.status, i + r.count);
  }

  // Read year-month delimiter
  i += r.count;
  if (str[i] != '-') {
    return result(EXESS_EXPECTED_DASH, i);
  }

  // Read month
  ++i;
  if ((r = read_two_digit_number(&out->month, 1, 12, str + i)).status) {
    return result(r.status, i + r.count);
  }

  // Read month-day delimiter
  i += r.count;
  if (str[i] != '-') {
    return result(EXESS_EXPECTED_DASH, i);
  }

  // Read day
  ++i;
  if ((r = read_two_digit_number(&out->day, 1, 31, str + i)).status) {
    return result(r.status, i + r.count);
  }

  // Check that day is in range
  i += r.count;
  if (out->day > days_in_month(out->year, out->month)) {
    return result(EXESS_OUT_OF_RANGE, i);
  }

  return result(EXESS_SUCCESS, i);
}

ExessResult
exess_read_date(ExessDate* const out, const char* const str)
{
  memset(out, 0, sizeof(*out));

  // Read YYYY-MM-DD numbers
  size_t      i = skip_whitespace(str);
  ExessResult r = read_date_numbers(out, str + i);

  i += r.count;
  if (r.status || is_end(str[i])) {
    out->zone = EXESS_LOCAL;
    return result(r.status, i);
  }

  // Read timezone
  r = exess_read_timezone(&out->zone, str + i);

  return result(r.status, i + r.count);
}

ExessResult
exess_write_date(const ExessDate value, const size_t buf_size, char* const buf)
{
  if (value.month < 1 || value.month > 12 || value.day < 1 || value.day > 31) {
    return end_write(EXESS_BAD_VALUE, buf_size, buf, 0);
  }

  ExessResult r = write_year_number(value.year, buf_size, buf);
  size_t      o = r.count;
  if (r.status) {
    return end_write(r.status, buf_size, buf, o);
  }

  o += write_char('-', buf_size, buf, o);
  o += write_two_digit_number(value.month, buf_size, buf, o);
  o += write_char('-', buf_size, buf, o);
  o += write_two_digit_number(value.day, buf_size, buf, o);

  r = write_timezone(value.zone, buf_size, buf, o);

  return end_write(r.status, buf_size, buf, o + r.count);
}
