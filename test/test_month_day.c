// Copyright 2011-2026 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#undef NDEBUG

#include "time_test_utils.h"

#include <exess/exess.h>

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

static const ExessMonthDay nozone   = {1, 2, EXESS_TIMEZONE_LOCAL};
static const ExessMonthDay utc      = {2, 3, EXESS_TIMEZONE_UTC};
static const ExessMonthDay zoned    = {3, 4, INIT_ZONE(11, 30)};
static const ExessMonthDay lowest   = {1, 1, INIT_ZONE(-14, 0)};
static const ExessMonthDay highest  = {12, 31, INIT_ZONE(14, 0)};
static const ExessMonthDay garbage1 = {0, 1, INIT_ZONE(11, 30)};
static const ExessMonthDay garbage2 = {13, 1, INIT_ZONE(11, 30)};
static const ExessMonthDay garbage3 = {1, 0, INIT_ZONE(11, 30)};
static const ExessMonthDay garbage4 = {1, 32, INIT_ZONE(11, 30)};

static void
check_read(const char* const string,
           const ExessStatus expected_status,
           const uint8_t     expected_month,
           const uint8_t     expected_day,
           const int8_t      expected_tz_hour,
           const int8_t      expected_tz_minute,
           const bool        expected_tz_is_present,
           const size_t      expected_count)
{
  ExessMonthDay value = {0, 0, EXESS_TIMEZONE_LOCAL};

  const ExessResult r = exess_read_month_day(string, &value);
  assert(r.status == expected_status);
  assert(r.count == expected_count);
  assert(value.month == expected_month);
  assert(value.day == expected_day);
  assert((!expected_tz_is_present && value.zone == EXESS_TIMEZONE_LOCAL) ||
         value.zone == (4 * expected_tz_hour) + (expected_tz_minute / 15));
}

static void
test_read_month_day(void)
{
  // No input
  check_read("", EXESS_EXPECTED_DASH, 0, 0, 0, 0, false, 0);
  check_read("\t\n\r ", EXESS_EXPECTED_DASH, 0, 0, 0, 0, false, 4);

  // Good values
  check_read("--04-12", EXESS_SUCCESS, 4, 12, 0, 0, false, 7);
  check_read("--04-12-05:00", EXESS_SUCCESS, 4, 12, -5, 0, true, 13);
  check_read("--04-12Z", EXESS_SUCCESS, 4, 12, 0, 0, true, 8);
  check_read("--10-26", EXESS_SUCCESS, 10, 26, 0, 0, false, 7);
  check_read("--10-26+02:00", EXESS_SUCCESS, 10, 26, 2, 0, true, 13);
  check_read("--10-26Z", EXESS_SUCCESS, 10, 26, 0, 0, true, 8);
  check_read("--10-26+00:00", EXESS_SUCCESS, 10, 26, 0, 0, true, 13);

  // Non-canonical
  check_read(" --04-12 ", EXESS_SUCCESS, 4, 12, 0, 0, false, 8);

  // Longest possible string
  check_read("--01-01-14:00",
             EXESS_SUCCESS,
             1,
             1,
             -14,
             0,
             true,
             EXESS_MAX_MONTH_DAY_LENGTH);

  // Limits
  check_read("--01-01", EXESS_SUCCESS, 1, 1, 0, 0, false, 7);
  check_read("--12-31", EXESS_SUCCESS, 12, 31, 0, 0, false, 7);

  // Out of range months
  check_read("--00-26", EXESS_OUT_OF_RANGE, 0, 0, 0, 0, false, 7);
  check_read("--13-26", EXESS_OUT_OF_RANGE, 13, 0, 0, 0, false, 7);

  // Out of range days (only roughly since the year is unknown)
  check_read("--02-00", EXESS_OUT_OF_RANGE, 2, 00, 0, 0, false, 7);
  check_read("--01-32", EXESS_OUT_OF_RANGE, 1, 32, 0, 0, false, 7);

  // Trailing garbage
  check_read("--04-12A", EXESS_SUCCESS, 4, 12, 0, 0, false, 7);

  // Garbage
  check_read("f", EXESS_EXPECTED_DASH, 0, 0, 0, 0, false, 0);
  check_read("--4-2", EXESS_EXPECTED_DIGIT, 4, 0, 0, 0, false, 5);
  check_read("/04/02", EXESS_EXPECTED_DASH, 0, 0, 0, 0, false, 0);
  check_read("-/04/02", EXESS_EXPECTED_DASH, 0, 0, 0, 0, false, 1);
}

static void
check_write(const ExessMonthDay value,
            const ExessStatus   expected_status,
            const size_t        buf_size,
            const char* const   expected_string)
{
  char buf[EXESS_MAX_MONTH_DAY_LENGTH + 1] = {
    1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};

  assert(buf_size <= sizeof(buf));

  const ExessResult r = exess_write_month_day(value, buf_size, buf);
  assert(r.status == expected_status);
  if (!r.status) {
    assert(!strcmp(buf, expected_string));
    assert(r.count == strlen(buf));
    assert(exess_write_month_day(value, 0, NULL).count == r.count);
  }
}

static void
test_write_month_day(void)
{
  check_write(nozone, EXESS_SUCCESS, 8, "--01-02");
  check_write(utc, EXESS_SUCCESS, 9, "--02-03Z");
  check_write(zoned, EXESS_SUCCESS, 14, "--03-04+11:30");
  check_write(lowest, EXESS_SUCCESS, 14, "--01-01-14:00");
  check_write(highest, EXESS_SUCCESS, 14, "--12-31+14:00");

  check_write(garbage1, EXESS_BAD_VALUE, 14, "");
  check_write(garbage2, EXESS_BAD_VALUE, 14, "");
  check_write(garbage3, EXESS_BAD_VALUE, 14, "");
  check_write(garbage4, EXESS_BAD_VALUE, 14, "");

  check_write(nozone, EXESS_NO_SPACE, 7, "");
  check_write(utc, EXESS_NO_SPACE, 8, "");
  check_write(zoned, EXESS_NO_SPACE, 13, "");
  check_write(lowest, EXESS_NO_SPACE, 3, "");
  check_write(highest, EXESS_NO_SPACE, 3, "");

  // Check that nothing is written when there isn't enough space
  char              c = 42;
  const ExessResult r = exess_write_month_day(nozone, 0, &c);
  assert(c == 42);
  assert(r.status == EXESS_NO_SPACE);
  assert(r.count == 7);
}

int
main(void)
{
  test_read_month_day();
  test_write_month_day();

  return 0;
}
