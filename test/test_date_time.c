// Copyright 2011-2025 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#undef NDEBUG

#include "write_test_utils.h"

#include <exess/exess.h>

#include <assert.h>
#include <stdint.h>
#include <string.h>

static const ExessDateTime local   = {2001, 2, 3, 56, 4, 5, 6, 0};
static const ExessDateTime utc     = {2001, 2, 3, EXESS_UTC, 4, 5, 6, 0};
static const ExessDateTime lowest  = {INT16_MIN, 1, 1, -56, 0, 0, 0, 0};
static const ExessDateTime highest = {INT16_MAX, 12, 31, 56, 24, 0, 0, 0};

static const ExessDateTime longest =
  {INT16_MIN, 12, 31, 56, 23, 59, 59, 999999999};

static const ExessDateTime utc_min = {INT16_MIN, 1, 1, EXESS_UTC, 0, 0, 0, 0};
static const ExessDateTime utc_max =
  {INT16_MAX, 12, 31, EXESS_UTC, 24, 0, 0, 0};

static const ExessDateTime nano      = {2001, 1, 1, 56, 0, 0, 0, 1};
static const ExessDateTime garbage1  = {2004, 0, 1, 56, 12, 0, 0, 0};
static const ExessDateTime garbage2  = {2005, 13, 1, 56, 12, 0, 0, 0};
static const ExessDateTime garbage3  = {2006, 1, 0, 56, 12, 0, 0, 0};
static const ExessDateTime garbage4  = {2006, 1, 32, 56, 12, 0, 0, 0};
static const ExessDateTime garbage5  = {2001, 2, 3, 56, 0, 0, 0, 1000000000};
static const ExessDateTime garbage6  = {2001, 2, 3, 56, 0, 0, 60, 0};
static const ExessDateTime garbage7  = {2001, 2, 3, 56, 0, 60, 0, 0};
static const ExessDateTime garbage8  = {2001, 2, 3, 56, 24, 0, 0, 1};
static const ExessDateTime garbage9  = {2001, 2, 3, 56, 24, 0, 1, 0};
static const ExessDateTime garbage10 = {2001, 2, 3, 56, 24, 1, 0, 0};
static const ExessDateTime garbage11 = {2001, 2, 3, 56, 25, 0, 0, 0};

static void
check_add(const char* const datetime_string,
          const char* const duration_string,
          const char* const result_string)
{
  ExessDateTime datetime = {0, 0U, 0U, EXESS_LOCAL, 0U, 0U, 0U, 0U};
  ExessDuration duration = {0U, 0U, 0U};

  ExessResult r = exess_read_date_time(&datetime, datetime_string);
  assert(!r.status);

  r = exess_read_duration(&duration, duration_string);
  assert(!r.status);

  const ExessDateTime result = exess_add_date_time_duration(datetime, duration);

  char buf[EXESS_MAX_DATE_TIME_LENGTH + 1] = {42};
  init_out_buf(sizeof(buf), buf);

  r = exess_write_date_time(result, sizeof(buf), buf);
  assert(!r.status);
  assert(!strcmp(buf, result_string));
}

static void
check_is_underflow(const ExessDateTime datetime, const ExessTimezone zone)
{
  assert(datetime.year == INT16_MIN);
  assert(datetime.month == 0);
  assert(datetime.day == 0);
  assert(datetime.zone == zone);
  assert(datetime.hour == 0);
  assert(datetime.minute == 0);
  assert(datetime.second == 0);
  assert(datetime.nanosecond == 0);
}

static void
check_is_overflow(const ExessDateTime datetime, const ExessTimezone zone)
{
  assert(datetime.year == INT16_MAX);
  assert(datetime.month == UINT8_MAX);
  assert(datetime.day == UINT8_MAX);
  assert(datetime.zone == zone);
  assert(datetime.hour == UINT8_MAX);
  assert(datetime.minute == UINT8_MAX);
  assert(datetime.second == UINT8_MAX);
  assert(datetime.nanosecond == UINT32_MAX);
}

static void
test_add(void)
{
  // Simple cases
  check_add("2001-01-01T00:00:00", "PT1.5S", "2001-01-01T00:00:01.5");
  check_add("2001-01-01T00:00:00", "PT1M", "2001-01-01T00:01:00");
  check_add("2001-01-01T00:00:00", "PT1H", "2001-01-01T01:00:00");
  check_add("2001-01-01T00:00:00", "P1D", "2001-01-02T00:00:00");
  check_add("2001-01-01T00:00:00", "P1M", "2001-02-01T00:00:00");
  check_add("2001-01-01T00:00:00", "P1Y", "2002-01-01T00:00:00");
  check_add("2001-02-02T02:02:02", "-PT1.5S", "2001-02-02T02:02:00.5");
  check_add("2001-02-02T02:02:02", "-PT1M", "2001-02-02T02:01:02");
  check_add("2001-02-02T02:02:02", "-PT1H", "2001-02-02T01:02:02");
  check_add("2001-02-02T02:02:02", "-P1D", "2001-02-01T02:02:02");
  check_add("2001-02-02T02:02:02", "-P1M", "2001-01-02T02:02:02");
  check_add("2001-02-02T02:02:02", "-P1Y", "2000-02-02T02:02:02");

  // Positive carrying
  check_add("2001-01-01T00:00:59", "PT1S", "2001-01-01T00:01:00");
  check_add("2001-01-01T00:59:00", "PT1M", "2001-01-01T01:00:00");
  check_add("2001-01-01T23:00:00", "PT1H", "2001-01-02T00:00:00");
  check_add("2001-01-31T00:00:00", "P1D", "2001-02-01T00:00:00");
  check_add("2001-12-01T00:00:00", "P1M", "2002-01-01T00:00:00");

  // Negative carrying
  check_add("2001-01-01T00:01:00", "-PT1S", "2001-01-01T00:00:59");
  check_add("2001-02-01T01:00:00", "-PT1M", "2001-02-01T00:59:00");
  check_add("2001-02-02T00:00:00", "-PT1H", "2001-02-01T23:00:00");
  check_add("2001-02-01T00:00:00", "-P1D", "2001-01-31T00:00:00");
  check_add("2001-01-01T00:00:00", "-P1M", "2000-12-01T00:00:00");

  // Underflow and overflow

  static const ExessDuration minus_month      = {-1, 0, 0};
  static const ExessDuration minus_second     = {0, -1, 0};
  static const ExessDuration minus_nanosecond = {0, 0, -1};
  static const ExessDuration plus_month       = {1, 0, 0};
  static const ExessDuration plus_second      = {0, 1, 0};
  static const ExessDuration plus_nanosecond  = {0, 0, 1};

  check_is_underflow(exess_add_date_time_duration(lowest, minus_month),
                     lowest.zone);
  check_is_underflow(exess_add_date_time_duration(lowest, minus_second),
                     lowest.zone);
  check_is_underflow(exess_add_date_time_duration(lowest, minus_nanosecond),
                     lowest.zone);

  check_is_underflow(exess_add_date_time_duration(utc_min, minus_month),
                     utc_min.zone);
  check_is_underflow(exess_add_date_time_duration(utc_min, minus_second),
                     utc_min.zone);
  check_is_underflow(exess_add_date_time_duration(utc_min, minus_nanosecond),
                     utc_min.zone);

  check_is_overflow(exess_add_date_time_duration(highest, plus_month),
                    highest.zone);
  check_is_overflow(exess_add_date_time_duration(highest, plus_second),
                    highest.zone);
  check_is_overflow(exess_add_date_time_duration(highest, plus_nanosecond),
                    highest.zone);

  check_is_overflow(exess_add_date_time_duration(utc_max, plus_month),
                    utc_max.zone);
  check_is_overflow(exess_add_date_time_duration(utc_max, plus_second),
                    utc_max.zone);
  check_is_overflow(exess_add_date_time_duration(utc_max, plus_nanosecond),
                    utc_max.zone);
}

static void
test_calendar(void)
{
  // February has 28 days in non-leap years
  check_add("-1400-02-28T12:00:00", "P1D", "-1400-03-01T12:00:00");
  check_add("-0399-02-28T12:00:00", "P1D", "-0399-03-01T12:00:00");
  check_add("-0300-02-28T12:00:00", "P1D", "-0300-03-01T12:00:00");
  check_add("-0200-02-28T12:00:00", "P1D", "-0200-03-01T12:00:00");
  check_add("-0100-02-28T12:00:00", "P1D", "-0100-03-01T12:00:00");
  check_add("-0001-02-28T12:00:00", "P1D", "-0001-03-01T12:00:00");
  check_add("0001-02-28T12:00:00", "P1D", "0001-03-01T12:00:00");
  check_add("0100-02-28T12:00:00", "P1D", "0100-03-01T12:00:00");
  check_add("0200-02-28T12:00:00", "P1D", "0200-03-01T12:00:00");
  check_add("0300-02-28T12:00:00", "P1D", "0300-03-01T12:00:00");
  check_add("0399-02-28T12:00:00", "P1D", "0399-03-01T12:00:00");
  check_add("1400-02-28T12:00:00", "P1D", "1400-03-01T12:00:00");

  // February has 29 days in leap years
  check_add("-0400-02-28T12:00:00", "P1D", "-0400-02-29T12:00:00");
  check_add("-0096-02-28T12:00:00", "P1D", "-0096-02-29T12:00:00");
  check_add("-0004-02-28T12:00:00", "P1D", "-0004-02-29T12:00:00");
  check_add("0000-02-28T12:00:00", "P1D", "0000-02-29T12:00:00");
  check_add("0004-02-28T12:00:00", "P1D", "0004-02-29T12:00:00");
  check_add("0096-02-28T12:00:00", "P1D", "0096-02-29T12:00:00");
  check_add("0400-02-28T12:00:00", "P1D", "0400-02-29T12:00:00");
  check_add("1600-02-28T12:00:00", "P1D", "1600-02-29T12:00:00");
}

static void
check_to_utc(const char* const datetime_string, const char* const result_string)
{
  ExessDateTime datetime = {0, 0U, 0U, EXESS_LOCAL, 0U, 0U, 0U, 0U};

  ExessResult r = exess_read_date_time(&datetime, datetime_string);
  assert(!r.status);

  const ExessDateTime result = exess_date_time_to_utc(datetime);

  char buf[EXESS_MAX_DATE_TIME_LENGTH + 1] = {42};
  init_out_buf(sizeof(buf), buf);

  r = exess_write_date_time(result, sizeof(buf), buf);
  assert(!r.status);
  assert(!strcmp(buf, result_string));
}

static void
test_to_utc(void)
{
  // Zoned
  check_to_utc("2001-02-03T04:46:59-00:15", "2001-02-03T05:01:59Z");

  // Positive carry: minute => hour => day
  check_to_utc("2001-02-03T23:46:59-00:15", "2001-02-04T00:01:59Z");

  // Positive carry: minute => hour => day => month (common year)
  check_to_utc("2001-02-28T23:46:59-00:15", "2001-03-01T00:01:59Z");

  // Positive carry: minute => hour => day => month (leap year)
  check_to_utc("2000-02-29T23:46:59-00:15", "2000-03-01T00:01:59Z");

  // Positive carry: minute => hour => day => month => year
  check_to_utc("2001-12-31T23:46:59-00:15", "2002-01-01T00:01:59Z");

  // Negative carry: minute => hour
  check_to_utc("2001-02-03T04:14:59+00:15", "2001-02-03T03:59:59Z");

  // Negative carry: minute => hour => day
  check_to_utc("2001-02-02T00:14:59+00:15", "2001-02-01T23:59:59Z");

  // Negative carry: minute => hour => day => month (common year)
  check_to_utc("2001-03-01T00:14:59+00:15", "2001-02-28T23:59:59Z");

  // Negative carry: minute => hour => day => month (leap year)
  check_to_utc("2000-03-01T00:14:59+00:15", "2000-02-29T23:59:59Z");

  // Negative carry: minute => hour => day => month => year
  check_to_utc("2001-01-01T00:14:59+00:15", "2000-12-31T23:59:59Z");
}

static void
check_read(const char* const   string,
           const ExessStatus   expected_status,
           const size_t        expected_count,
           const int64_t       expected_year,
           const uint8_t       expected_month,
           const uint8_t       expected_day,
           const uint8_t       expected_hour,
           const uint8_t       expected_minute,
           const uint8_t       expected_second,
           const uint32_t      expected_nanosecond,
           const ExessTimezone expected_zone)
{
  ExessDateTime value = {0, 0, 0, EXESS_LOCAL, 0, 0, 0, 0};

  const ExessResult r = exess_read_date_time(&value, string);
  assert(r.status == expected_status);
  assert(r.count == expected_count);
  assert(value.year == expected_year);
  assert(value.month == expected_month);
  assert(value.day == expected_day);
  assert(value.hour == expected_hour);
  assert(value.minute == expected_minute);
  assert(value.second == expected_second);
  assert(value.nanosecond == expected_nanosecond);
  assert(value.zone == expected_zone);
}

static void
test_read_date_time(void)
{
  // Simple values

  check_read("2001-02-03T04:05:06",
             EXESS_SUCCESS,
             19,
             2001,
             2,
             3,
             4,
             5,
             6,
             0,
             EXESS_LOCAL);

  check_read("2001-02-03T04:05:06Z",
             EXESS_SUCCESS,
             20,
             2001,
             2,
             3,
             4,
             5,
             6,
             0,
             EXESS_UTC);

  check_read("2004-04-12T13:20:15.5",
             EXESS_SUCCESS,
             21,
             2004,
             4,
             12,
             13,
             20,
             15,
             500000000,
             EXESS_LOCAL);

  // Time zone offsets

  check_read("-32768-01-01T00:00:00.000000001+14:00",
             EXESS_SUCCESS,
             EXESS_MAX_DATE_TIME_LENGTH,
             -32768,
             1,
             1,
             0,
             0,
             0,
             1,
             exess_timezone(14, 0));

  check_read("2001-02-03T04:05:06.7-08:00",
             EXESS_SUCCESS,
             27,
             2001,
             2,
             3,
             4,
             5,
             6,
             700000000,
             exess_timezone(-8, 0));

  // Midnight
  check_read("2001-02-03T24:00:00",
             EXESS_SUCCESS,
             19,
             2001,
             2,
             3,
             24,
             0,
             0,
             0,
             EXESS_LOCAL);

  // Garbage

  check_read("2004-04-12T13:00",
             EXESS_EXPECTED_COLON,
             16,
             2004,
             4,
             12,
             13,
             0,
             0,
             0,
             EXESS_LOCAL);

  check_read(
    "2004-04-121", EXESS_EXPECTED_TIME_SEP, 10, 2004, 4, 12, 0, 0, 0, 0, 0);

  check_read(
    "99-04-12T13:00", EXESS_EXPECTED_DIGIT, 2, 99, 0, 0, 0, 0, 0, 0, 0);

  check_read(
    "2004-04-12", EXESS_EXPECTED_TIME_SEP, 10, 2004, 4, 12, 0, 0, 0, 0, 0);

  check_read("2004-04-12-05:00",
             EXESS_EXPECTED_TIME_SEP,
             10,
             2004,
             4,
             12,
             0,
             0,
             0,
             0,
             0);
}

static void
check_write(const ExessDateTime value,
            const ExessStatus   expected_status,
            const size_t        buf_size,
            const char* const   expected_string)
{
  char buf[EXESS_MAX_DATE_TIME_LENGTH + 1] = {42};
  init_out_buf(sizeof(buf), buf);

  assert(buf_size <= sizeof(buf));

  const ExessResult r = exess_write_date_time(value, buf_size, buf);
  assert(r.status == expected_status);
  assert(r.count == strlen(buf));
  assert(!strcmp(buf, expected_string));
  assert(r.status || exess_write_date_time(value, 0, NULL).count == r.count);
}

static void
test_write_date_time(void)
{
  check_write(local, EXESS_SUCCESS, 26, "2001-02-03T04:05:06+14:00");
  check_write(utc, EXESS_SUCCESS, 21, "2001-02-03T04:05:06Z");
  check_write(lowest, EXESS_SUCCESS, 28, "-32768-01-01T00:00:00-14:00");
  check_write(highest, EXESS_SUCCESS, 27, "32767-12-31T24:00:00+14:00");
  check_write(
    longest, EXESS_SUCCESS, 38, "-32768-12-31T23:59:59.999999999+14:00");
  check_write(nano, EXESS_SUCCESS, 36, "2001-01-01T00:00:00.000000001+14:00");

  check_write(garbage1, EXESS_BAD_VALUE, 38, "");
  check_write(garbage2, EXESS_BAD_VALUE, 38, "");
  check_write(garbage3, EXESS_BAD_VALUE, 38, "");
  check_write(garbage4, EXESS_BAD_VALUE, 38, "");
  check_write(garbage5, EXESS_BAD_VALUE, 38, "");
  check_write(garbage6, EXESS_BAD_VALUE, 38, "");
  check_write(garbage7, EXESS_BAD_VALUE, 38, "");
  check_write(garbage8, EXESS_BAD_VALUE, 38, "");
  check_write(garbage9, EXESS_BAD_VALUE, 38, "");
  check_write(garbage10, EXESS_BAD_VALUE, 38, "");
  check_write(garbage11, EXESS_BAD_VALUE, 38, "");

  check_write(lowest, EXESS_NO_SPACE, 12, "");
  check_write(lowest, EXESS_NO_SPACE, 17, "");
  check_write(lowest, EXESS_NO_SPACE, 18, "");
  check_write(lowest, EXESS_NO_SPACE, 21, "");

  // Check that nothing is written when there isn't enough space
  char              c = 42;
  const ExessResult r = exess_write_date_time(highest, 0, &c);
  assert(c == 42);
  assert(r.status == EXESS_NO_SPACE);
  assert(r.count == 0);
}

int
main(void)
{
  test_add();
  test_calendar();
  test_to_utc();
  test_read_date_time();
  test_write_date_time();

  return 0;
}
