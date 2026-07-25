// Copyright 2019-2026 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#include "date_utils.h"
#include "number_utils.h"
#include "read_utils.h"
#include "result.h"
#include "timezone.h"
#include "write_utils.h"
#include "year.h"

#include <exess/exess.h>

#include <string.h>

EXESS_NONBLOCKING ExessOrder
exess_compare_date(const ExessDate lhs, const ExessDate rhs)
{
  const ExessDateTime lhs_datetime = {
    lhs.year, lhs.month, lhs.day, lhs.zone, 0, 0, 0, 0};

  const ExessDateTime rhs_datetime = {
    rhs.year, rhs.month, rhs.day, rhs.zone, 0, 0, 0, 0};

  return exess_compare_date_time(lhs_datetime, rhs_datetime);
}

EXESS_NONBLOCKING ExessResult
exess_read_date(const char* const str, ExessDate* const out)
{
  memset(out, 0, sizeof(*out));

  // Read YYYY-MM-DD numbers
  size_t      i = skip_whitespace(str);
  ExessResult r = read_date_numbers(out, str + i);

  i += r.count;

  // Read timezone if present
  if (!r.status) {
    r = read_optional_timezone(&out->zone, str + i);
    i += r.count;
  }

  return RESULT(r.status, i);
}

EXESS_NONBLOCKING ExessResult
exess_write_date(const ExessDate value, const size_t buf_size, char* const buf)
{
  if (value.month < 1 || value.month > 12 || value.day < 1 || value.day > 31) {
    return end_write(EXESS_BAD_VALUE, buf_size, buf, 0);
  }

  // Write YYYY-MM-DD numbers
  ExessResult r = write_year_number(value.year, buf_size, buf);
  r.count += write_char('-', buf_size, buf, r.count);
  r.count += write_two_digit_number(value.month, buf_size, buf, r.count);
  r.count += write_char('-', buf_size, buf, r.count);
  r.count += write_two_digit_number(value.day, buf_size, buf, r.count);

  // Write timezone suffix and end write
  return write_timezone(value.zone, buf_size, buf, r);
}
