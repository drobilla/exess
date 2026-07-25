// Copyright 2019-2026 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#include "date_utils.h"
#include "number_utils.h"
#include "result.h"
#include "year.h"

#include <exess/exess.h>

#include <stddef.h>

EXESS_NONBLOCKING ExessResult
read_date_m(uint8_t* const m_out, const char* const str)
{
  // Skip year-month delimiter
  if (str[0] != '-') {
    return RESULT(EXESS_EXPECTED_DASH, 0);
  }

  // Read month
  const ExessResult r = read_two_digit_number(m_out, 1, 12, str + 1U);
  return RESULT(r.status, 1U + r.count);
}

EXESS_NONBLOCKING ExessResult
read_date_ym(int16_t* const y_out, uint8_t* const m_out, const char* const str)
{
  // Read leading year
  ExessResult r = read_year_number(y_out, str);
  if (r.status) {
    return r;
  }

  size_t i = r.count;
  r        = read_date_m(m_out, str + i);
  r.count  = i + r.count;
  return r;
}

EXESS_NONBLOCKING ExessResult
read_date_d(uint8_t* const d_out, const char* const str)
{
  // Skip month-day delimiter
  if (str[0] != '-') {
    return RESULT(EXESS_EXPECTED_DASH, 0);
  }

  // Read day
  ExessResult r = read_two_digit_number(d_out, 1, 31, str + 1U);
  r.count       = 1U + r.count;
  return r;
}

EXESS_NONBLOCKING ExessResult
read_date_numbers(ExessDate* const out, const char* const str)
{
  // Read year and month
  ExessResult r = read_date_ym(&out->year, &out->month, str);

  if (!r.status) {
    // Read (month-day delimiter and) day
    size_t i = r.count;
    r        = read_date_d(&out->day, str + i);
    r.count  = i + r.count;
  }

  // Check that day is in range
  if (!r.status && out->day > days_in_month(out->year, out->month)) {
    r.status = EXESS_OUT_OF_RANGE;
  }

  return r;
}
