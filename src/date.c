// Copyright 2019-2025 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#include "date_utils.h"
#include "read_utils.h"
#include "result.h"
#include "timezone.h"
#include "write_utils.h"
#include "year.h"

#include <exess/exess.h>

#include <string.h>

int
exess_compare_date(const ExessDate lhs, const ExessDate rhs)
{
  const ExessDateTime lhs_datetime = {
    lhs.year, lhs.month, lhs.day, lhs.zone, 0, 0, 0, 0};

  const ExessDateTime rhs_datetime = {
    rhs.year, rhs.month, rhs.day, rhs.zone, 0, 0, 0, 0};

  return exess_compare_date_time(lhs_datetime, rhs_datetime);
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
  r = read_timezone(&out->zone, str + i);

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
