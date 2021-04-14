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

#undef NDEBUG

#include "time_test_utils.h"

#include "exess/exess.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include <stdio.h>

static const ExessTimezone missing  = {EXESS_LOCAL};
static const ExessTimezone utc      = INIT_ZONE(0, 0);
static const ExessTimezone plus     = INIT_ZONE(11, 30);
static const ExessTimezone minus    = INIT_ZONE(-11, -30);
static const ExessTimezone slight   = INIT_ZONE(0, 30);
static const ExessTimezone lowest   = INIT_ZONE(-14, 0);
static const ExessTimezone highest  = INIT_ZONE(14, 0);
static const ExessTimezone garbage1 = INIT_ZONE(-14, -15);
static const ExessTimezone garbage2 = INIT_ZONE(14, 15);
static const ExessTimezone garbage3 = INIT_ZONE(-15, 0);
static const ExessTimezone garbage4 = INIT_ZONE(15, 0);

static void
check_read(const char* const string,
           const ExessStatus expected_status,
           const int8_t      expected_hour,
           const int8_t      expected_minute,
           const bool        expected_is_present,
           const size_t      expected_count)
{
  // The timezone interface is not public, so we test it via time
  char time_string[] = "12:00:00XXXXXX";
  strncpy(time_string + 8, string, sizeof(time_string) - 9);

  ExessTime         value = {{0}, 0, 0, 0, 0};
  const ExessResult r     = exess_read_time(&value, time_string);

  assert(r.status == expected_status);
  assert(r.count == 8 + expected_count);
  assert((!expected_is_present && value.zone.quarter_hours == EXESS_LOCAL) ||
         value.zone.quarter_hours == 4 * expected_hour + expected_minute / 15);
}

static void
test_read_timezone(void)
{
  // Basic values
  check_read("Z", EXESS_SUCCESS, 0, 0, true, 1);
  check_read("-05:00", EXESS_SUCCESS, -5, 0, true, 6);
  check_read("+02:00", EXESS_SUCCESS, 2, 0, true, 6);
  check_read("+00:00", EXESS_SUCCESS, 0, 0, true, 6);
  check_read("-00:00", EXESS_SUCCESS, 0, 0, true, 6);

  // Limits
  check_read("-14:00", EXESS_SUCCESS, -14, 0, true, 6);
  check_read("+14:00", EXESS_SUCCESS, 14, 0, true, 6);
  check_read("-13:45", EXESS_SUCCESS, -13, -45, true, 6);
  check_read("+13:45", EXESS_SUCCESS, 13, 45, true, 6);

  // Out of range
  check_read("-14:15", EXESS_OUT_OF_RANGE, 0, 0, false, 6);
  check_read("+14:15", EXESS_OUT_OF_RANGE, 0, 0, false, 6);
  check_read("-15:00", EXESS_OUT_OF_RANGE, 0, 0, false, 3);
  check_read("+15:00", EXESS_OUT_OF_RANGE, 0, 0, false, 3);
  check_read("-13:60", EXESS_OUT_OF_RANGE, 0, 0, false, 6);
  check_read("+13:60", EXESS_OUT_OF_RANGE, 0, 0, false, 6);

  // Garbage
  check_read("+05:01", EXESS_UNSUPPORTED, 0, 0, false, 6);
  check_read("05:00", EXESS_EXPECTED_SIGN, 0, 0, false, 0);
  check_read("+5:00", EXESS_EXPECTED_DIGIT, 0, 0, false, 2);
  check_read("+5:0", EXESS_EXPECTED_DIGIT, 0, 0, false, 2);
  check_read("+5:", EXESS_EXPECTED_DIGIT, 0, 0, false, 2);
  check_read("+:0", EXESS_EXPECTED_DIGIT, 0, 0, false, 1);
  check_read("+A5:00", EXESS_EXPECTED_DIGIT, 0, 0, false, 1);
  check_read("+0A:00", EXESS_EXPECTED_DIGIT, 0, 0, false, 2);
  check_read("+05A00", EXESS_EXPECTED_COLON, 0, 0, false, 3);
  check_read("+05:A0", EXESS_EXPECTED_DIGIT, 0, 0, false, 4);
  check_read("+05:0A", EXESS_EXPECTED_DIGIT, 0, 0, false, 5);
}

static void
check_write(const ExessTimezone value,
            const ExessStatus   expected_status,
            const size_t        buf_size,
            const char* const   expected_string)
{
  // The timezone interface is not public, so we test it via time
  char buf[EXESS_MAX_TIME_LENGTH + 1] = {1,  2,  3,  4,  5,  6,  7,  8,  9,
                                         10, 11, 12, 13, 14, 15, 16, 17, 18,
                                         19, 20, 21, 22, 23, 24, 25};

  assert(buf_size <= sizeof(buf) - 8);

  const ExessTime   time = {value, 12, 0, 0, 0};
  const ExessResult r    = exess_write_time(time, 8 + buf_size, buf);
  assert(r.status == expected_status);
  assert(r.count == strlen(buf));
  assert((!buf[0] && !expected_string[0]) || !strcmp(buf + 8, expected_string));
  assert(r.status || exess_write_time(time, 0, NULL).count == r.count);
}

static void
test_write_timezone(void)
{
  check_write(missing, EXESS_SUCCESS, 1, "");
  check_write(utc, EXESS_SUCCESS, 2, "Z");
  check_write(plus, EXESS_SUCCESS, 7, "+11:30");
  check_write(minus, EXESS_SUCCESS, 7, "-11:30");
  check_write(slight, EXESS_SUCCESS, 7, "+00:30");
  check_write(lowest, EXESS_SUCCESS, 7, "-14:00");
  check_write(highest, EXESS_SUCCESS, 7, "+14:00");

  check_write(garbage1, EXESS_BAD_VALUE, 7, "");
  check_write(garbage2, EXESS_BAD_VALUE, 7, "");
  check_write(garbage3, EXESS_BAD_VALUE, 7, "");
  check_write(garbage4, EXESS_BAD_VALUE, 7, "");

  check_write(utc, EXESS_NO_SPACE, 1, "");
  check_write(plus, EXESS_NO_SPACE, 6, "");
}

static void
check_round_trip(const ExessTimezone value)
{
  ExessTime parsed_time                    = {{0}, 0, 0, 0, 0};
  char      buf[EXESS_MAX_TIME_LENGTH + 1] = {42};

  const ExessTime time = {value, 12, 0, 0, 0};

  assert(!exess_write_time(time, sizeof(buf), buf).status);
  assert(!exess_read_time(&parsed_time, buf).status);
  assert(!memcmp(&parsed_time.zone, &value, sizeof(ExessTimezone)));
}

static void
test_round_trip(void)
{
  check_round_trip(lowest);
  check_round_trip(highest);

  for (int8_t h = -13; h < 13; ++h) {
    for (int8_t q = 0; q < 4; ++q) {
      const ExessTimezone value = {(int8_t)(4 * h + q)};

      check_round_trip(value);
    }
  }
}

int
main(void)
{
  test_read_timezone();
  test_write_timezone();
  test_round_trip();

  return 0;
}
