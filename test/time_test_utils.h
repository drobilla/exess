/*
  Copyright 2011-2021 David Robillard <d@drobilla.net>

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

#include "int_test_data.h"

#include "exess/exess.h"

#include <stdint.h>

#define INIT_ZONE(hour, minute) \
  {                             \
    4 * (hour) + (minute) / 15  \
  }

static inline ExessTimezone
random_timezone(uint32_t* rng)
{
  *rng = lcg32(*rng);

  const int8_t hour = (int8_t)((*rng % 27) - 13);

  *rng = lcg32(*rng);

  const int8_t minute = (int8_t)((hour < 0 ? -1 : 1) * (int32_t)(*rng % 60));

  const ExessTimezone zone = {(int8_t)(4 * hour + minute / 15)};
  return zone;
}

static inline bool
timezone_matches(const ExessTimezone zone,
                 const int8_t        expected_hour,
                 const int8_t        expected_minute,
                 const bool          expected_is_present)

{
  return (!expected_is_present && zone.quarter_hours == EXESS_LOCAL) ||
         zone.quarter_hours == 4 * expected_hour + expected_minute / 15;
}
