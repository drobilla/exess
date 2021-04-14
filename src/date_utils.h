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

#ifndef EXESS_DATE_UTILS_H
#define EXESS_DATE_UTILS_H

#include "exess/exess.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

static inline bool
is_leap_year(const int64_t year)
{
  if (year % 4) {
    return false;
  }

  if (year % 100) {
    return true;
  }

  if (year % 400) {
    return false;
  }

  return true;
}

static inline uint8_t
days_in_month(const int16_t year, const uint8_t month)
{
  return month == 2u ? (is_leap_year(year) ? 29u : 28u)
                     : (uint8_t)(30u + (month + (month / 8u)) % 2u);
}

ExessResult
read_year_number(int16_t* out, const char* str);

ExessResult
write_year_number(int16_t value, size_t buf_size, char* buf);

/// Read YYYY-MM-DD date numbers without a timezone
ExessResult
read_date_numbers(ExessDate* out, const char* str);

#endif // EXESS_DATE_UTILS_H
