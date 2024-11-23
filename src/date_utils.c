// Copyright 2019-2023 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#include "date_utils.h"
#include "read_utils.h"
#include "result.h"
#include "year.h"

#include "exess/exess.h"

#include <stddef.h>

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
  r = read_two_digit_number(&out->month, 1, 12, str + i);
  if (r.status) {
    return result(r.status, i + r.count);
  }

  // Read month-day delimiter
  i += r.count;
  if (str[i] != '-') {
    return result(EXESS_EXPECTED_DASH, i);
  }

  // Read day
  ++i;
  r = read_two_digit_number(&out->day, 1, 31, str + i);
  if (r.status) {
    return result(r.status, i + r.count);
  }

  // Check that day is in range
  i += r.count;
  if (out->day > days_in_month(out->year, out->month)) {
    return result(EXESS_OUT_OF_RANGE, i);
  }

  return result(EXESS_SUCCESS, i);
}
