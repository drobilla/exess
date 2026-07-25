// Copyright 2011-2026 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#undef NDEBUG

#include "time_test_utils.h"

#include <exess/exess.h>

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

static const ExessYearMonth nozone   = {2001, 1, EXESS_TIMEZONE_LOCAL};
static const ExessYearMonth utc      = {2002, 2, EXESS_TIMEZONE_UTC};
static const ExessYearMonth zoned    = {2003, 3, INIT_ZONE(11, 30)};
static const ExessYearMonth early    = {99, 3, INIT_ZONE(11, 30)};
static const ExessYearMonth future   = {12345, 3, INIT_ZONE(11, 30)};
static const ExessYearMonth lowest   = {INT16_MIN, 1, INIT_ZONE(-14, 0)};
static const ExessYearMonth highest  = {INT16_MAX, 1, INIT_ZONE(14, 0)};
static const ExessYearMonth garbage1 = {2004, 0, INIT_ZONE(11, 30)};
static const ExessYearMonth garbage2 = {2005, 13, INIT_ZONE(11, 30)};

static void
check_read(const char* const string,
           const ExessStatus expected_status,
           const int64_t     expected_year,
           const uint8_t     expected_month,
           const int8_t      expected_tz_hour,
           const int8_t      expected_tz_minute,
           const bool        expected_tz_is_present,
           const size_t      expected_count)
{
  ExessYearMonth value = {0, 0, EXESS_TIMEZONE_LOCAL};

  const ExessResult r = exess_read_year_month(string, &value);
  assert(r.status == expected_status);
  assert(r.count == expected_count);
  assert(value.year == expected_year);
  assert(value.month == expected_month);
  assert((!expected_tz_is_present && value.zone == EXESS_TIMEZONE_LOCAL) ||
         value.zone == (4 * expected_tz_hour) + (expected_tz_minute / 15));
}

static void
test_read_year_month(void)
{
  // No input
  check_read("", EXESS_EXPECTED_DIGIT, 0, 0, 0, 0, false, 0);
  check_read("\t\n\r ", EXESS_EXPECTED_DIGIT, 0, 0, 0, 0, false, 4);

  // Good values
  check_read("2004-04", EXESS_SUCCESS, 2004, 4, 0, 0, false, 7);
  check_read("-0045-01", EXESS_SUCCESS, -45, 1, 0, 0, false, 8);
  check_read("12004-04", EXESS_SUCCESS, 12004, 4, 0, 0, false, 8);
  check_read("2004-04-05:00", EXESS_SUCCESS, 2004, 4, -5, 0, true, 13);
  check_read("2004-04Z", EXESS_SUCCESS, 2004, 4, 0, 0, true, 8);
  check_read("2001-10", EXESS_SUCCESS, 2001, 10, 0, 0, false, 7);
  check_read("2001-10+02:00", EXESS_SUCCESS, 2001, 10, 2, 0, true, 13);
  check_read("2001-10Z", EXESS_SUCCESS, 2001, 10, 0, 0, true, 8);
  check_read("2001-10+00:00", EXESS_SUCCESS, 2001, 10, 0, 0, true, 13);
  check_read("-2001-10", EXESS_SUCCESS, -2001, 10, 0, 0, false, 8);
  check_read("-20000-04", EXESS_SUCCESS, -20000, 04, 0, 0, false, 9);

  // Non-canonical
  check_read("02004-04", EXESS_SUCCESS, 2004, 4, 0, 0, false, 8);
  check_read(" 02004-04 ", EXESS_SUCCESS, 2004, 4, 0, 0, false, 9);

  // Longest possible string
  check_read("-32768-01-14:00",
             EXESS_SUCCESS,
             -32768,
             1,
             -14,
             0,
             true,
             EXESS_MAX_YEAR_MONTH_LENGTH);

  // Limits
  check_read("-32768-01", EXESS_SUCCESS, -32768, 1, 0, 0, false, 9);
  check_read("32767-01", EXESS_SUCCESS, 32767, 1, 0, 0, false, 8);

  // Out of range years
  check_read("-32769-01", EXESS_OUT_OF_RANGE, 0, 0, 0, 0, false, 6);
  check_read("32768-01", EXESS_OUT_OF_RANGE, 0, 0, 0, 0, false, 5);

  // Out of range months
  check_read("2001-00", EXESS_OUT_OF_RANGE, 2001, 0, 0, 0, false, 7);
  check_read("2001-13", EXESS_OUT_OF_RANGE, 2001, 13, 0, 0, false, 7);

  // Trailing garbage
  check_read("2004-04A", EXESS_SUCCESS, 2004, 4, 0, 0, false, 7);

  // Garbage
  check_read("f", EXESS_EXPECTED_DIGIT, 0, 0, 0, 0, false, 0);
  check_read("99-04", EXESS_EXPECTED_DIGIT, 99, 0, 0, 0, false, 2);
  check_read("2004-4", EXESS_EXPECTED_DIGIT, 2004, 4, 0, 0, false, 6);
  check_read("2004/04", EXESS_EXPECTED_DASH, 2004, 0, 0, 0, false, 4);
  check_read("04-12", EXESS_EXPECTED_DIGIT, 4, 0, 0, 0, false, 2);
}

static void
check_write(const ExessYearMonth value,
            const ExessStatus    expected_status,
            const size_t         buf_size,
            const char* const    expected_string)
{
  char buf[EXESS_MAX_YEAR_MONTH_LENGTH + 1] = {
    1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};

  assert(buf_size <= sizeof(buf));

  const ExessResult r = exess_write_year_month(value, buf_size, buf);
  assert(r.status == expected_status);
  if (!r.status) {
    assert(!strcmp(buf, expected_string));
    assert(r.count == strlen(buf));
    assert(exess_write_year_month(value, 0, NULL).count == r.count);
  }
}

static void
test_write_year_month(void)
{
  check_write(nozone, EXESS_SUCCESS, 8, "2001-01");
  check_write(utc, EXESS_SUCCESS, 9, "2002-02Z");
  check_write(zoned, EXESS_SUCCESS, 14, "2003-03+11:30");
  check_write(early, EXESS_SUCCESS, 14, "0099-03+11:30");
  check_write(future, EXESS_SUCCESS, 15, "12345-03+11:30");
  check_write(lowest, EXESS_SUCCESS, 16, "-32768-01-14:00");
  check_write(highest, EXESS_SUCCESS, 15, "32767-01+14:00");

  check_write(garbage1, EXESS_BAD_VALUE, 14, "");
  check_write(garbage2, EXESS_BAD_VALUE, 14, "");

  check_write(nozone, EXESS_NO_SPACE, 7, "");
  check_write(utc, EXESS_NO_SPACE, 8, "");
  check_write(zoned, EXESS_NO_SPACE, 13, "");
  check_write(early, EXESS_NO_SPACE, 13, "");
  check_write(future, EXESS_NO_SPACE, 5, "");
  check_write(lowest, EXESS_NO_SPACE, 5, "");
  check_write(highest, EXESS_NO_SPACE, 5, "");

  // Check that nothing is written when there isn't enough space
  char              c = 42;
  const ExessResult r = exess_write_year_month(nozone, 0, &c);
  assert(c == 42);
  assert(r.status == EXESS_NO_SPACE);
  assert(r.count == 7);
}

int
main(void)
{
  test_read_year_month();
  test_write_year_month();

  return 0;
}
