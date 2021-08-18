// Copyright 2011-2021 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#include "int_test_data.h"

#include "exess/exess.h"

#include <stdbool.h>
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
