// Copyright 2011-2021 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#undef NDEBUG

#include "date_utils.h"
#include "int_test_data.h"
#include "macros.h"
#include "num_test_utils.h"
#include "time_test_utils.h"

#include "exess/exess.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static const ExessDate nozone   = {2001, 1, 2, EXESS_LOCAL};
static const ExessDate utc      = {2002, 2, 3, EXESS_UTC};
static const ExessDate zoned    = {2003, 3, 4, INIT_ZONE(11, 30)};
static const ExessDate early    = {99, 3, 4, INIT_ZONE(11, 30)};
static const ExessDate future   = {12345, 3, 4, INIT_ZONE(11, 30)};
static const ExessDate lowest   = {INT16_MIN, 1, 1, INIT_ZONE(-14, 0)};
static const ExessDate highest  = {INT16_MAX, 1, 1, INIT_ZONE(14, 0)};
static const ExessDate garbage1 = {2004, 0, 1, INIT_ZONE(11, 30)};
static const ExessDate garbage2 = {2005, 13, 1, INIT_ZONE(11, 30)};
static const ExessDate garbage3 = {2006, 1, 0, INIT_ZONE(11, 30)};
static const ExessDate garbage4 = {2006, 1, 32, INIT_ZONE(11, 30)};

static void
check_read(const char* const string,
           const ExessStatus expected_status,
           const int64_t     expected_year,
           const uint8_t     expected_month,
           const uint8_t     expected_day,
           const int8_t      expected_tz_hour,
           const int8_t      expected_tz_minute,
           const bool        expected_tz_is_present,
           const size_t      expected_count)
{
  ExessDate value = {0, 0, 0, EXESS_LOCAL};

  const ExessResult r = exess_read_date(&value, string);
  assert(r.status == expected_status);
  assert(r.count == expected_count);
  assert(value.year == expected_year);
  assert(value.month == expected_month);
  assert(value.day == expected_day);
  assert((!expected_tz_is_present && value.zone == EXESS_LOCAL) ||
         value.zone == 4 * expected_tz_hour + expected_tz_minute / 15);
}

static void
test_read_date(void)
{
  // No input
  check_read("", EXESS_EXPECTED_DIGIT, 0, 0, 0, 0, 0, false, 0);
  check_read(" \f\n\r\t\v", EXESS_EXPECTED_DIGIT, 0, 0, 0, 0, 0, false, 6);

  // Good values
  check_read("2004-04-12", EXESS_SUCCESS, 2004, 4, 12, 0, 0, false, 10);
  check_read("-0045-01-01", EXESS_SUCCESS, -45, 1, 1, 0, 0, false, 11);
  check_read("12004-04-12", EXESS_SUCCESS, 12004, 4, 12, 0, 0, false, 11);
  check_read("2004-04-12-05:00", EXESS_SUCCESS, 2004, 4, 12, -5, 0, true, 16);
  check_read("2004-04-12Z", EXESS_SUCCESS, 2004, 4, 12, 0, 0, true, 11);
  check_read("2001-10-26", EXESS_SUCCESS, 2001, 10, 26, 0, 0, false, 10);
  check_read("2001-10-26+02:00", EXESS_SUCCESS, 2001, 10, 26, 2, 0, true, 16);
  check_read("2001-10-26Z", EXESS_SUCCESS, 2001, 10, 26, 0, 0, true, 11);
  check_read("2001-10-26+00:00", EXESS_SUCCESS, 2001, 10, 26, 0, 0, true, 16);
  check_read("-2001-10-26", EXESS_SUCCESS, -2001, 10, 26, 0, 0, false, 11);
  check_read("-20000-04-01", EXESS_SUCCESS, -20000, 04, 01, 0, 0, false, 12);

  // Non-canonical
  check_read("02004-04-12", EXESS_SUCCESS, 2004, 4, 12, 0, 0, false, 11);
  check_read(" 02004-04-12 ", EXESS_SUCCESS, 2004, 4, 12, 0, 0, false, 12);

  // Good common year values
  check_read("1900-02-28", EXESS_SUCCESS, 1900, 2, 28, 0, 0, false, 10);

  // Good leap year values
  check_read("2000-02-29", EXESS_SUCCESS, 2000, 2, 29, 0, 0, false, 10);
  check_read("2004-02-29", EXESS_SUCCESS, 2004, 2, 29, 0, 0, false, 10);

  // Longest possible string
  check_read("-32768-01-01-14:00",
             EXESS_SUCCESS,
             -32768,
             1,
             1,
             -14,
             0,
             true,
             EXESS_MAX_DATE_LENGTH);

  // Limits
  check_read("-32768-01-01", EXESS_SUCCESS, -32768, 1, 1, 0, 0, false, 12);
  check_read("32767-01-01", EXESS_SUCCESS, 32767, 1, 1, 0, 0, false, 11);

  // Out of range years
  check_read("-32769-01-01", EXESS_OUT_OF_RANGE, 0, 0, 0, 0, 0, false, 6);
  check_read("32768-01-01", EXESS_OUT_OF_RANGE, 0, 0, 0, 0, 0, false, 5);

  // Out of range months
  check_read("2001-00-26", EXESS_OUT_OF_RANGE, 2001, 0, 0, 0, 0, false, 7);
  check_read("2001-13-26", EXESS_OUT_OF_RANGE, 2001, 13, 0, 0, 0, false, 7);
  check_read("2001-10-00", EXESS_OUT_OF_RANGE, 2001, 10, 0, 0, 0, false, 10);
  check_read("2001-10-32", EXESS_OUT_OF_RANGE, 2001, 10, 32, 0, 0, false, 10);

  // Out of range days
  check_read("2001-01-32", EXESS_OUT_OF_RANGE, 2001, 1, 32, 0, 0, false, 10);
  check_read("2001-02-29", EXESS_OUT_OF_RANGE, 2001, 2, 29, 0, 0, false, 10);
  check_read("2001-03-32", EXESS_OUT_OF_RANGE, 2001, 3, 32, 0, 0, false, 10);
  check_read("2001-04-31", EXESS_OUT_OF_RANGE, 2001, 4, 31, 0, 0, false, 10);
  check_read("2001-05-32", EXESS_OUT_OF_RANGE, 2001, 5, 32, 0, 0, false, 10);
  check_read("2001-06-31", EXESS_OUT_OF_RANGE, 2001, 6, 31, 0, 0, false, 10);
  check_read("2001-07-32", EXESS_OUT_OF_RANGE, 2001, 7, 32, 0, 0, false, 10);
  check_read("2001-08-32", EXESS_OUT_OF_RANGE, 2001, 8, 32, 0, 0, false, 10);
  check_read("2001-09-31", EXESS_OUT_OF_RANGE, 2001, 9, 31, 0, 0, false, 10);
  check_read("2001-10-32", EXESS_OUT_OF_RANGE, 2001, 10, 32, 0, 0, false, 10);
  check_read("2001-11-31", EXESS_OUT_OF_RANGE, 2001, 11, 31, 0, 0, false, 10);
  check_read("2001-12-32", EXESS_OUT_OF_RANGE, 2001, 12, 32, 0, 0, false, 10);

  // Garbage
  check_read("f", EXESS_EXPECTED_DIGIT, 0, 0, 0, 0, 0, false, 0);
  check_read("99-04-12", EXESS_EXPECTED_DIGIT, 99, 0, 0, 0, 0, false, 2);
  check_read("2004-4-2", EXESS_EXPECTED_DIGIT, 2004, 4, 0, 0, 0, false, 6);
  check_read("2004/04/02", EXESS_EXPECTED_DASH, 2004, 0, 0, 0, 0, false, 4);
  check_read("04-12-2004", EXESS_EXPECTED_DIGIT, 4, 0, 0, 0, 0, false, 2);
  check_read("2001-10", EXESS_EXPECTED_DASH, 2001, 10, 0, 0, 0, false, 7);
  check_read("01-10-26", EXESS_EXPECTED_DIGIT, 1, 0, 0, 0, 0, false, 2);
  check_read("2004-04-12A", EXESS_EXPECTED_SIGN, 2004, 4, 12, 0, 0, false, 10);
}

static void
check_write(const ExessDate   value,
            const ExessStatus expected_status,
            const size_t      buf_size,
            const char* const expected_string)
{
  char buf[EXESS_MAX_DATE_LENGTH + 1] = {
    1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19};

  assert(buf_size <= sizeof(buf));

  const ExessResult r = exess_write_date(value, buf_size, buf);
  assert(r.status == expected_status);
  assert(r.count == strlen(buf));
  assert(!strcmp(buf, expected_string));
  assert(r.status || exess_write_date(value, 0, NULL).count == r.count);
}

static void
test_write_date(void)
{
  check_write(nozone, EXESS_SUCCESS, 11, "2001-01-02");
  check_write(utc, EXESS_SUCCESS, 12, "2002-02-03Z");
  check_write(zoned, EXESS_SUCCESS, 17, "2003-03-04+11:30");
  check_write(early, EXESS_SUCCESS, 17, "0099-03-04+11:30");
  check_write(future, EXESS_SUCCESS, 18, "12345-03-04+11:30");
  check_write(lowest, EXESS_SUCCESS, 19, "-32768-01-01-14:00");
  check_write(highest, EXESS_SUCCESS, 18, "32767-01-01+14:00");

  check_write(garbage1, EXESS_BAD_VALUE, 14, "");
  check_write(garbage2, EXESS_BAD_VALUE, 14, "");
  check_write(garbage3, EXESS_BAD_VALUE, 14, "");
  check_write(garbage4, EXESS_BAD_VALUE, 14, "");

  check_write(nozone, EXESS_NO_SPACE, 10, "");
  check_write(future, EXESS_NO_SPACE, 5, "");
  check_write(lowest, EXESS_NO_SPACE, 5, "");
  check_write(highest, EXESS_NO_SPACE, 5, "");
  check_write(utc, EXESS_NO_SPACE, 11, "");
  check_write(zoned, EXESS_NO_SPACE, 16, "");

  // Check that nothing is written when there isn't enough space
  char              c = 42;
  const ExessResult r = exess_write_date(nozone, 0, &c);
  assert(c == 42);
  assert(r.status == EXESS_NO_SPACE);
  assert(r.count == 0);
}

static void
check_round_trip(const ExessDate value)
{
  ExessDate parsed_value                   = {0, 0, 0, EXESS_LOCAL};
  char      buf[EXESS_MAX_DATE_LENGTH + 1] = {0};

  assert(!exess_write_date(value, sizeof(buf), buf).status);
  assert(!exess_read_date(&parsed_value, buf).status);
  assert(parsed_value.year == value.year);
  assert(parsed_value.month == value.month);
  assert(parsed_value.day == value.day);
  assert(parsed_value.zone == value.zone);
}

static void
test_round_trip(const ExessNumTestOptions opts)
{
  fprintf(stderr, "Testing xsd:gDate randomly with seed %u\n", opts.seed);

  const uint64_t n_tests = MAX(256, opts.n_tests / 16);

  uint32_t rng = opts.seed;
  for (uint64_t i = 0; i < n_tests; ++i) {
    rng = lcg32(rng);

    const int16_t year = (int16_t)(rng % UINT16_MAX);
    for (uint8_t month = 1; month < 13; ++month) {
      for (uint8_t day = 1; day <= days_in_month(year, month); ++day) {
        const ExessDate no_zone      = {year, month, day, EXESS_LOCAL};
        const ExessDate lowest_zone  = {year, month, day, 4 * -14 + 0};
        const ExessDate highest_zone = {year, month, day, 4 * 14};

        check_round_trip(no_zone);
        check_round_trip(lowest_zone);
        check_round_trip(highest_zone);

        const ExessDate value = {year, month, day, random_timezone(&rng)};
        check_round_trip(value);
      }
    }

    print_num_test_progress(i, n_tests);
  }
}

int
main(int argc, char** argv)
{
  const ExessNumTestOptions opts = parse_num_test_options(argc, argv);

  if (!opts.error) {
    test_read_date();
    test_write_date();
    test_round_trip(opts);
  }

  return (int)opts.error;
}
