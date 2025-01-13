// Copyright 2011-2021 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#undef NDEBUG

#include "double_test_utils.h"
#include "int_test_utils.h"
#include "time_test_utils.h"

#include <exess/exess.h>

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

static const ExessTime nozone   = {EXESS_LOCAL, 0, 0, 0, 0};
static const ExessTime utc      = {EXESS_UTC, 12, 15, 1, 250000000};
static const ExessTime zoned    = {INIT_ZONE(11, 30), 23, 59, 59, 1000000};
static const ExessTime high     = {INIT_ZONE(11, 30), 24, 0, 0, 0};
static const ExessTime garbage1 = {INIT_ZONE(11, 30), 0, 0, 0, 1000000000};
static const ExessTime garbage2 = {INIT_ZONE(11, 30), 0, 0, 60, 0};
static const ExessTime garbage3 = {INIT_ZONE(11, 30), 0, 60, 0, 0};
static const ExessTime garbage4 = {INIT_ZONE(11, 30), 24, 0, 0, 1};
static const ExessTime garbage5 = {INIT_ZONE(11, 30), 24, 0, 1, 0};
static const ExessTime garbage6 = {INIT_ZONE(11, 30), 24, 1, 0, 0};
static const ExessTime garbage7 = {INIT_ZONE(11, 30), 25, 0, 0, 0};

static void
check_read(const char* const string,
           const ExessStatus expected_status,
           const uint8_t     expected_hour,
           const uint8_t     expected_minute,
           const uint8_t     expected_second,
           const uint32_t    expected_nanosecond,
           const int8_t      expected_tz_hour,
           const int8_t      expected_tz_minute,
           const bool        expected_tz_is_present,
           const size_t      expected_count)
{
  ExessTime value = {EXESS_LOCAL, 0, 0, 0, 0};

  const ExessResult r = exess_read_time(&value, string);
  assert(r.status == expected_status);
  assert(r.count == expected_count);
  assert(value.hour == expected_hour);
  assert(value.minute == expected_minute);
  assert(value.second == expected_second);
  assert(value.nanosecond == expected_nanosecond);
  assert((!expected_tz_is_present && value.zone == EXESS_LOCAL) ||
         value.zone == 4 * expected_tz_hour + expected_tz_minute / 15);
}

static void
test_read_time(void)
{
  // No value
  check_read("", EXESS_EXPECTED_DIGIT, 0, 0, 0, 0, 0, 0, false, 0);
  check_read(" \f\n\r\t\v", EXESS_EXPECTED_DIGIT, 0, 0, 0, 0, 0, 0, false, 6);

  // Good values
  check_read("13:20:00", EXESS_SUCCESS, 13, 20, 0, 0, 0, 0, false, 8);
  check_read(
    "13:20:30.5555", EXESS_SUCCESS, 13, 20, 30, 555500000, 0, 0, false, 13);
  check_read("13:20:00-05:00", EXESS_SUCCESS, 13, 20, 0, 0, -5, 0, true, 14);
  check_read("13:20:00Z", EXESS_SUCCESS, 13, 20, 0, 0, 0, 0, true, 9);
  check_read("00:00:00", EXESS_SUCCESS, 0, 0, 0, 0, 0, 0, false, 8);
  check_read("24:00:00", EXESS_SUCCESS, 24, 0, 0, 0, 0, 0, false, 8);
  check_read("21:32:52", EXESS_SUCCESS, 21, 32, 52, 0, 0, 0, false, 8);
  check_read("21:32:52+02:00", EXESS_SUCCESS, 21, 32, 52, 0, 2, 0, true, 14);
  check_read("19:32:52Z", EXESS_SUCCESS, 19, 32, 52, 0, 0, 0, true, 9);
  check_read("19:32:52+00:00", EXESS_SUCCESS, 19, 32, 52, 0, 0, 0, true, 14);
  check_read(
    "21:32:52.12679", EXESS_SUCCESS, 21, 32, 52, 126790000, 0, 0, false, 14);

  // Longest possible string
  check_read("24:59:59.000000001-14:00",
             EXESS_SUCCESS,
             24,
             59,
             59,
             1,
             -14,
             0,
             true,
             EXESS_MAX_TIME_LENGTH);

  // Non-canonical form
  check_read(
    " \f\n\r\t\v13:20:00 ", EXESS_SUCCESS, 13, 20, 0, 0, 0, 0, false, 14);

  // Trailing whitespace
  check_read("13:20:45 ", EXESS_SUCCESS, 13, 20, 45, 0, 0, 0, false, 8);
  check_read("13:20:45\f", EXESS_SUCCESS, 13, 20, 45, 0, 0, 0, false, 8);
  check_read("13:20:45\n", EXESS_SUCCESS, 13, 20, 45, 0, 0, 0, false, 8);
  check_read("13:20:45\r", EXESS_SUCCESS, 13, 20, 45, 0, 0, 0, false, 8);
  check_read("13:20:45\t", EXESS_SUCCESS, 13, 20, 45, 0, 0, 0, false, 8);
  check_read("13:20:45\v", EXESS_SUCCESS, 13, 20, 45, 0, 0, 0, false, 8);

  // Trailing garbage
  check_read("13:20:00junk", EXESS_EXPECTED_SIGN, 13, 20, 0, 0, 0, 0, false, 8);
  check_read("13:20:00Zjunk", EXESS_EXPECTED_END, 13, 20, 0, 0, 0, 0, true, 9);

  // Too many nanosecond digits (expecting a timezone after 9 digits)
  check_read("01:02:03.123456789123",
             EXESS_EXPECTED_SIGN,
             1,
             2,
             3,
             123456789,
             0,
             0,
             false,
             18);

  // Garbage
  check_read("13.20.00", EXESS_EXPECTED_COLON, 13, 0, 0, 0, 0, 0, false, 2);
  check_read("13:20:", EXESS_EXPECTED_DIGIT, 13, 20, 0, 0, 0, 0, false, 6);
  check_read("5:20:00", EXESS_EXPECTED_DIGIT, 5, 0, 0, 0, 0, 0, false, 1);
  check_read("13:20", EXESS_EXPECTED_COLON, 13, 20, 0, 0, 0, 0, false, 5);
  check_read("13:20.5:00", EXESS_EXPECTED_COLON, 13, 20, 0, 0, 0, 0, false, 5);
  check_read("13:65:00", EXESS_OUT_OF_RANGE, 13, 65, 0, 0, 0, 0, false, 5);
  check_read("21:32", EXESS_EXPECTED_COLON, 21, 32, 0, 0, 0, 0, false, 5);
  check_read("25:25:10", EXESS_OUT_OF_RANGE, 25, 0, 0, 0, 0, 0, false, 2);
  check_read("-10:00:00", EXESS_EXPECTED_DIGIT, 0, 0, 0, 0, 0, 0, false, 0);
  check_read("1:20:10", EXESS_EXPECTED_DIGIT, 1, 0, 0, 0, 0, 0, false, 1);
  check_read("13:20:00A", EXESS_EXPECTED_SIGN, 13, 20, 0, 0, 0, 0, false, 8);
}

static void
check_write(const ExessTime   value,
            const ExessStatus expected_status,
            const size_t      buf_size,
            const char* const expected_string)
{
  char buf[EXESS_MAX_TIME_LENGTH + 1] = {1,  2,  3,  4,  5,  6,  7,  8,  9,
                                         10, 11, 12, 13, 14, 15, 16, 17, 18,
                                         19, 20, 21, 22, 23, 24, 0};

  assert(buf_size <= sizeof(buf));

  const ExessResult r = exess_write_time(value, buf_size, buf);
  assert(r.status == expected_status);
  assert(r.count == strlen(buf));
  assert(!strcmp(buf, expected_string));
  assert(r.status || exess_write_time(value, 0, NULL).count == r.count);
}

static void
test_write_time(void)
{
  check_write(nozone, EXESS_SUCCESS, 9, "00:00:00");
  check_write(utc, EXESS_SUCCESS, 13, "12:15:01.25Z");
  check_write(zoned, EXESS_SUCCESS, 19, "23:59:59.001+11:30");
  check_write(high, EXESS_SUCCESS, 15, "24:00:00+11:30");

  check_write(garbage1, EXESS_BAD_VALUE, 19, "");
  check_write(garbage2, EXESS_BAD_VALUE, 19, "");
  check_write(garbage3, EXESS_BAD_VALUE, 19, "");
  check_write(garbage4, EXESS_BAD_VALUE, 19, "");
  check_write(garbage5, EXESS_BAD_VALUE, 19, "");
  check_write(garbage6, EXESS_BAD_VALUE, 19, "");
  check_write(garbage7, EXESS_BAD_VALUE, 19, "");

  check_write(nozone, EXESS_NO_SPACE, 8, "");
  check_write(utc, EXESS_NO_SPACE, 12, "");
  check_write(zoned, EXESS_NO_SPACE, 18, "");
  check_write(zoned, EXESS_NO_SPACE, 12, "");
  check_write(high, EXESS_NO_SPACE, 14, "");

  // Check that nothing is written when there isn't enough space
  char              c = 42;
  const ExessResult r = exess_write_time(nozone, 0, &c);
  assert(c == 42);
  assert(r.status == EXESS_NO_SPACE);
  assert(r.count == 0);
}

static void
check_round_trip(const ExessTime value)
{
  ExessTime parsed_value                   = {EXESS_LOCAL, 0, 0, 0, 0};
  char      buf[EXESS_MAX_TIME_LENGTH + 1] = {0};

  assert(!exess_write_time(value, sizeof(buf), buf).status);
  assert(!exess_read_time(&parsed_value, buf).status);
  assert(parsed_value.hour == value.hour);
  assert(parsed_value.minute == value.minute);
  assert(double_matches(parsed_value.second, value.second));
  assert(parsed_value.zone == value.zone);
}

static void
test_round_trip(void)
{
  uint32_t rng = 0;
  for (uint8_t h = 0; h < 24; ++h) {
    for (uint8_t m = 0; m < 60; ++m) {
      rng = lcg32(rng);

      const uint32_t ns = rng % 1000000000U;

      rng = lcg32(rng);

      const uint8_t   s            = (uint8_t)(rng % 60U);
      const ExessTime no_zone      = {EXESS_LOCAL, h, m, s, ns};
      const ExessTime lowest_zone  = {INIT_ZONE(-14, 0), h, m, s, ns};
      const ExessTime highest_zone = {INIT_ZONE(14, 0), h, m, s, ns};

      check_round_trip(no_zone);
      check_round_trip(lowest_zone);
      check_round_trip(highest_zone);

      const ExessTime value = {random_timezone(&rng), h, m, s, ns};
      check_round_trip(value);
    }
  }
}

int
main(void)
{
  test_read_time();
  test_write_time();
  test_round_trip();

  return 0;
}
