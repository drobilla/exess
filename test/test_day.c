// Copyright 2011-2026 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#undef NDEBUG

#include "time_test_utils.h"

#include <exess/exess.h>

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

static const ExessDay nozone   = {2, EXESS_TIMEZONE_LOCAL};
static const ExessDay utc      = {3, EXESS_TIMEZONE_UTC};
static const ExessDay zoned    = {4, INIT_ZONE(11, 30)};
static const ExessDay lowest   = {1, INIT_ZONE(-14, 0)};
static const ExessDay highest  = {31, INIT_ZONE(14, 0)};
static const ExessDay garbage1 = {0, INIT_ZONE(11, 30)};
static const ExessDay garbage2 = {32, INIT_ZONE(11, 30)};

static void
check_read(const char* const string,
           const ExessStatus expected_status,
           const uint8_t     expected_day,
           const int8_t      expected_tz_hour,
           const int8_t      expected_tz_minute,
           const bool        expected_tz_is_present,
           const size_t      expected_count)
{
  ExessDay value = {0, EXESS_TIMEZONE_LOCAL};

  const ExessResult r = exess_read_day(string, &value);
  assert(r.status == expected_status);
  assert(r.count == expected_count);
  assert(value.day == expected_day);
  assert((!expected_tz_is_present && value.zone == EXESS_TIMEZONE_LOCAL) ||
         value.zone == (4 * expected_tz_hour) + (expected_tz_minute / 15));
}

static void
test_read_day(void)
{
  // No input
  check_read("", EXESS_EXPECTED_DASH, 0, 0, 0, false, 0);
  check_read("\t\n\r ", EXESS_EXPECTED_DASH, 0, 0, 0, false, 4);

  // Good values
  check_read("---12", EXESS_SUCCESS, 12, 0, 0, false, 5);
  check_read("---01", EXESS_SUCCESS, 1, 0, 0, false, 5);
  check_read("---12-05:00", EXESS_SUCCESS, 12, -5, 0, true, 11);
  check_read("---12Z", EXESS_SUCCESS, 12, 0, 0, true, 6);
  check_read("---26", EXESS_SUCCESS, 26, 0, 0, false, 5);
  check_read("---26+02:00", EXESS_SUCCESS, 26, 2, 0, true, 11);
  check_read("---26Z", EXESS_SUCCESS, 26, 0, 0, true, 6);
  check_read("---26+00:00", EXESS_SUCCESS, 26, 0, 0, true, 11);

  // Non-canonical
  check_read(" ---12 ", EXESS_SUCCESS, 12, 0, 0, false, 6);

  // Longest possible string
  check_read(
    "---01-14:00", EXESS_SUCCESS, 1, -14, 0, true, EXESS_MAX_DAY_LENGTH);

  // Limits
  check_read("---01", EXESS_SUCCESS, 1, 0, 0, false, 5);
  check_read("---31", EXESS_SUCCESS, 31, 0, 0, false, 5);

  // Out of range days
  check_read("---00", EXESS_OUT_OF_RANGE, 0, 0, 0, false, 5);
  check_read("---32", EXESS_OUT_OF_RANGE, 32, 0, 0, false, 5);

  // Trailing garbage
  check_read("---12A", EXESS_SUCCESS, 12, 0, 0, false, 5);

  // Garbage
  check_read("f", EXESS_EXPECTED_DASH, 0, 0, 0, false, 0);
  check_read("-12", EXESS_EXPECTED_DASH, 0, 0, 0, false, 1);
  check_read("--12", EXESS_EXPECTED_DASH, 0, 0, 0, false, 2);
  check_read("---", EXESS_EXPECTED_DIGIT, 0, 0, 0, false, 3);
  check_read("---1", EXESS_EXPECTED_DIGIT, 1, 0, 0, false, 4);
  check_read("/12", EXESS_EXPECTED_DASH, 0, 0, 0, false, 0);
}

static void
check_write(const ExessDay    value,
            const ExessStatus expected_status,
            const size_t      buf_size,
            const char* const expected_string)
{
  char buf[EXESS_MAX_DATE_LENGTH + 1] = {1, 2, 3, 4, 5, 6, 7, 8, 9};

  assert(buf_size <= sizeof(buf));

  const ExessResult r = exess_write_day(value, buf_size, buf);
  assert(r.status == expected_status);
  if (!r.status) {
    assert(!strcmp(buf, expected_string));
    assert(r.count == strlen(buf));
    assert(exess_write_day(value, 0, NULL).count == r.count);
  }
}

static void
test_write_day(void)
{
  check_write(nozone, EXESS_SUCCESS, 6, "---02");
  check_write(utc, EXESS_SUCCESS, 7, "---03Z");
  check_write(zoned, EXESS_SUCCESS, 12, "---04+11:30");
  check_write(lowest, EXESS_SUCCESS, 12, "---01-14:00");
  check_write(highest, EXESS_SUCCESS, 12, "---31+14:00");

  check_write(garbage1, EXESS_BAD_VALUE, 12, "");
  check_write(garbage2, EXESS_BAD_VALUE, 12, "");

  check_write(nozone, EXESS_NO_SPACE, 5, "");
  check_write(utc, EXESS_NO_SPACE, 6, "");
  check_write(zoned, EXESS_NO_SPACE, 11, "");
  check_write(lowest, EXESS_NO_SPACE, 11, "");
  check_write(highest, EXESS_NO_SPACE, 11, "");

  // Check that nothing is written when there isn't enough space
  char              c = 42;
  const ExessResult r = exess_write_day(nozone, 0, &c);
  assert(c == 42);
  assert(r.status == EXESS_NO_SPACE);
  assert(r.count == 5);
}

int
main(void)
{
  test_read_day();
  test_write_day();

  return 0;
}
