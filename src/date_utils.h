// Copyright 2019-2024 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#ifndef EXESS_SRC_DATE_UTILS_H
#define EXESS_SRC_DATE_UTILS_H

#include <exess/exess.h>

#include <stdbool.h>
#include <stdint.h>

/// Return whether a year is a leap year in the proleptic Gregorian calendar
static inline bool
is_leap_year(const int64_t year)
{
  return !(year % 4) && ((year % 100) || !(year % 400));
}

/// Return the number of days in some month of the proleptic Gregorian calendar
static inline uint8_t
days_in_month(const int16_t year, const uint8_t month)
{
  return month == 2U ? (is_leap_year(year) ? 29U : 28U)
                     : (uint8_t)(30U + (month + (month / 8U)) % 2U);
}

/// Read YYYY-MM-DD date numbers without a timezone
ExessResult
read_date_numbers(ExessDate* out, const char* str);

#endif // EXESS_SRC_DATE_UTILS_H
