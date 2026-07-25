// Copyright 2011-2026 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#undef NDEBUG

#include "time_test_utils.h"

#include <exess/exess.h>

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

static const ExessMonth nozone   = {1, EXESS_TIMEZONE_LOCAL};
static const ExessMonth utc      = {2, EXESS_TIMEZONE_UTC};
static const ExessMonth zoned    = {3, INIT_ZONE(11, 30)};
static const ExessMonth lowest   = {1, INIT_ZONE(-14, 0)};
static const ExessMonth highest  = {12, INIT_ZONE(14, 0)};
static const ExessMonth garbage1 = {0, INIT_ZONE(11, 30)};
static const ExessMonth garbage2 = {13, INIT_ZONE(11, 30)};

static void
check_read(const char* const string,
           const ExessStatus expected_status,
           const uint8_t     expected_month,
           const int8_t      expected_tz_hour,
           const int8_t      expected_tz_minute,
           const bool        expected_tz_is_present,
           const size_t      expected_count)
{
  ExessMonth value = {0, EXESS_TIMEZONE_LOCAL};

  const ExessResult r = exess_read_month(string, &value);
  assert(r.status == expected_status);
  assert(r.count == expected_count);
  assert(value.month == expected_month);
  assert((!expected_tz_is_present && value.zone == EXESS_TIMEZONE_LOCAL) ||
         value.zone == (4 * expected_tz_hour) + (expected_tz_minute / 15));
}

static void
test_read_month(void)
{
  // No input
  check_read("", EXESS_EXPECTED_DASH, 0, 0, 0, false, 0);
  check_read("\t\n\r ", EXESS_EXPECTED_DASH, 0, 0, 0, false, 4);

  // Good values
  check_read("--04", EXESS_SUCCESS, 4, 0, 0, false, 4);
  check_read("--04-05:00", EXESS_SUCCESS, 4, -5, 0, true, 10);
  check_read("--04Z", EXESS_SUCCESS, 4, 0, 0, true, 5);
  check_read("--10", EXESS_SUCCESS, 10, 0, 0, false, 4);
  check_read("--10+02:00", EXESS_SUCCESS, 10, 2, 0, true, 10);
  check_read("--10Z", EXESS_SUCCESS, 10, 0, 0, true, 5);
  check_read("--10+00:00", EXESS_SUCCESS, 10, 0, 0, true, 10);

  // Non-canonical
  check_read(" --04 ", EXESS_SUCCESS, 4, 0, 0, false, 5);

  // Longest possible string
  check_read(
    "--01-14:00", EXESS_SUCCESS, 1, -14, 0, true, EXESS_MAX_MONTH_LENGTH);

  // Limits
  check_read("--01", EXESS_SUCCESS, 1, 0, 0, false, 4);
  check_read("--12", EXESS_SUCCESS, 12, 0, 0, false, 4);

  // Out of range months
  check_read("--00", EXESS_OUT_OF_RANGE, 0, 0, 0, false, 4);
  check_read("--13", EXESS_OUT_OF_RANGE, 13, 0, 0, false, 4);

  // Trailing garbage
  check_read("--04A", EXESS_SUCCESS, 4, 12, 0, false, 4);

  // Garbage
  check_read("f", EXESS_EXPECTED_DASH, 0, 0, 0, false, 0);
  check_read("-4", EXESS_EXPECTED_DASH, 0, 0, 0, false, 1);
  check_read("--4", EXESS_EXPECTED_DIGIT, 4, 0, 0, false, 3);
  check_read("/04", EXESS_EXPECTED_DASH, 0, 0, 0, false, 0);
}

static void
check_write(const ExessMonth  value,
            const ExessStatus expected_status,
            const size_t      buf_size,
            const char* const expected_string)
{
  char buf[EXESS_MAX_MONTH_LENGTH + 1] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};

  assert(buf_size <= sizeof(buf));

  const ExessResult r = exess_write_month(value, buf_size, buf);
  assert(r.status == expected_status);
  if (!r.status) {
    assert(!strcmp(buf, expected_string));
    assert(r.count == strlen(buf));
    assert(exess_write_month(value, 0, NULL).count == r.count);
  }
}

static void
test_write_month(void)
{
  check_write(nozone, EXESS_SUCCESS, 5, "--01");
  check_write(utc, EXESS_SUCCESS, 6, "--02Z");
  check_write(zoned, EXESS_SUCCESS, 11, "--03+11:30");
  check_write(lowest, EXESS_SUCCESS, 11, "--01-14:00");
  check_write(highest, EXESS_SUCCESS, 11, "--12+14:00");

  check_write(garbage1, EXESS_BAD_VALUE, 11, "");
  check_write(garbage2, EXESS_BAD_VALUE, 11, "");

  check_write(nozone, EXESS_NO_SPACE, 4, "");
  check_write(utc, EXESS_NO_SPACE, 5, "");
  check_write(zoned, EXESS_NO_SPACE, 10, "");
  check_write(lowest, EXESS_NO_SPACE, 10, "");
  check_write(highest, EXESS_NO_SPACE, 10, "");

  // Check that nothing is written when there isn't enough space
  char              c = 42;
  const ExessResult r = exess_write_month(nozone, 0, &c);
  assert(c == 42);
  assert(r.status == EXESS_NO_SPACE);
  assert(r.count == 4);
}

int
main(void)
{
  test_read_month();
  test_write_month();

  return 0;
}
