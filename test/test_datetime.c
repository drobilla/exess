// Copyright 2011-2024 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#undef NDEBUG

#include "exess/exess.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

static const ExessDateTime local     = {2001, 2, 3, false, 4, 5, 6, 0};
static const ExessDateTime utc       = {2001, 2, 3, true, 4, 5, 6, 0};
static const ExessDateTime lowest    = {INT16_MIN, 1, 1, false, 0, 0, 0, 0};
static const ExessDateTime highest   = {INT16_MAX, 12, 31, false, 24, 0, 0, 0};
static const ExessDateTime utc_min   = {INT16_MIN, 1, 1, true, 0, 0, 0, 0};
static const ExessDateTime utc_max   = {INT16_MAX, 12, 31, true, 24, 0, 0, 0};
static const ExessDateTime nano      = {2001, 1, 1, false, 0, 0, 0, 1};
static const ExessDateTime garbage1  = {2004, 0, 1, false, 12, 0, 0, 0};
static const ExessDateTime garbage2  = {2005, 13, 1, false, 12, 0, 0, 0};
static const ExessDateTime garbage3  = {2006, 1, 0, false, 12, 0, 0, 0};
static const ExessDateTime garbage4  = {2006, 1, 32, false, 12, 0, 0, 0};
static const ExessDateTime garbage5  = {2001, 2, 3, false, 0, 0, 0, 1000000000};
static const ExessDateTime garbage6  = {2001, 2, 3, false, 0, 0, 60, 0};
static const ExessDateTime garbage7  = {2001, 2, 3, false, 0, 60, 0, 0};
static const ExessDateTime garbage8  = {2001, 2, 3, false, 24, 0, 0, 1};
static const ExessDateTime garbage9  = {2001, 2, 3, false, 24, 0, 1, 0};
static const ExessDateTime garbage10 = {2001, 2, 3, false, 24, 1, 0, 0};
static const ExessDateTime garbage11 = {2001, 2, 3, false, 25, 0, 0, 0};

static void
check_add(const char* const datetime_string,
          const char* const duration_string,
          const char* const result_string)
{
  ExessDateTime datetime = {0, 0U, 0U, false, 0U, 0U, 0U, 0U};
  ExessDuration duration = {0U, 0U, 0U};

  ExessResult r = exess_read_datetime(&datetime, datetime_string);
  assert(!r.status);

  r = exess_read_duration(&duration, duration_string);
  assert(!r.status);

  const ExessDateTime result  = exess_add_datetime_duration(datetime, duration);
  char                buf[28] = {0};

  r = exess_write_datetime(result, sizeof(buf), buf);
  assert(!r.status);
  assert(!strcmp(buf, result_string));
}

static void
check_is_underflow(const ExessDateTime datetime, const bool is_utc)
{
  assert(datetime.year == INT16_MIN);
  assert(datetime.month == 0);
  assert(datetime.day == 0);
  assert(datetime.is_utc == is_utc);
  assert(datetime.hour == 0);
  assert(datetime.minute == 0);
  assert(datetime.second == 0);
  assert(datetime.nanosecond == 0);
}

static void
check_is_overflow(const ExessDateTime datetime, const bool is_utc)
{
  assert(datetime.year == INT16_MAX);
  assert(datetime.month == UINT8_MAX);
  assert(datetime.day == UINT8_MAX);
  assert(datetime.is_utc == is_utc);
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

  check_is_underflow(exess_add_datetime_duration(lowest, minus_month), false);
  check_is_underflow(exess_add_datetime_duration(lowest, minus_second), false);
  check_is_underflow(exess_add_datetime_duration(lowest, minus_nanosecond),
                     false);

  check_is_underflow(exess_add_datetime_duration(utc_min, minus_month), true);
  check_is_underflow(exess_add_datetime_duration(utc_min, minus_second), true);
  check_is_underflow(exess_add_datetime_duration(utc_min, minus_nanosecond),
                     true);

  check_is_overflow(exess_add_datetime_duration(highest, plus_month), false);
  check_is_overflow(exess_add_datetime_duration(highest, plus_second), false);
  check_is_overflow(exess_add_datetime_duration(highest, plus_nanosecond),
                    false);

  check_is_overflow(exess_add_datetime_duration(utc_max, plus_month), true);
  check_is_overflow(exess_add_datetime_duration(utc_max, plus_second), true);
  check_is_overflow(exess_add_datetime_duration(utc_max, plus_nanosecond),
                    true);
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
check_read(const char* const string,
           const ExessStatus expected_status,
           const size_t      expected_count,
           const int64_t     expected_year,
           const uint8_t     expected_month,
           const uint8_t     expected_day,
           const uint8_t     expected_hour,
           const uint8_t     expected_minute,
           const uint8_t     expected_second,
           const uint32_t    expected_nanosecond,
           const bool        expected_is_utc)
{
  ExessDateTime value = {0, 0, 0, false, 0, 0, 0, 0};

  const ExessResult r = exess_read_datetime(&value, string);
  assert(r.status == expected_status);
  assert(r.count == expected_count);
  assert(value.year == expected_year);
  assert(value.month == expected_month);
  assert(value.day == expected_day);
  assert(value.hour == expected_hour);
  assert(value.minute == expected_minute);
  assert(value.second == expected_second);
  assert(value.nanosecond == expected_nanosecond);
  assert(value.is_utc == expected_is_utc);
}

static void
test_read_datetime(void)
{
  // Simple values

  check_read(
    "2001-02-03T04:05:06", EXESS_SUCCESS, 19, 2001, 2, 3, 4, 5, 6, 0, false);

  check_read(
    "2001-02-03T04:05:06Z", EXESS_SUCCESS, 20, 2001, 2, 3, 4, 5, 6, 0, true);

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
             false);

  check_read("-32768-01-01T00:00:00.000000001Z",
             EXESS_SUCCESS,
             EXESS_MAX_DATETIME_LENGTH,
             -32768,
             1,
             1,
             0,
             0,
             0,
             1,
             true);

  // Simple timezones

  check_read("2001-02-03T04:05:06-00:30",
             EXESS_SUCCESS,
             25,
             2001,
             2,
             3,
             4,
             35,
             6,
             0,
             true);

  check_read("2001-02-03T04:05:06-01:00",
             EXESS_SUCCESS,
             25,
             2001,
             2,
             3,
             5,
             5,
             6,
             0,
             true);

  check_read("2001-02-03T04:05:06+00:30",
             EXESS_SUCCESS,
             25,
             2001,
             2,
             3,
             3,
             35,
             6,
             0,
             true);

  check_read("2001-02-03T04:05:06+01:00",
             EXESS_SUCCESS,
             25,
             2001,
             2,
             3,
             3,
             5,
             6,
             0,
             true);

  // Positive timezone carry

  // Minute => hour
  check_read("2001-02-03T04:46:00-00:15",
             EXESS_SUCCESS,
             25,
             2001,
             2,
             3,
             5,
             1,
             0,
             0,
             true);

  // Minute => hour => day
  check_read("2001-02-03T23:46:00-00:15",
             EXESS_SUCCESS,
             25,
             2001,
             2,
             4,
             0,
             1,
             0,
             0,
             true);

  // Minute => hour => day => month
  check_read("2001-02-28T23:46:00-00:15",
             EXESS_SUCCESS,
             25,
             2001,
             3,
             1,
             0,
             1,
             0,
             0,
             true);

  // Minute => hour => day => month => year
  check_read("2001-12-31T23:46:00-00:15",
             EXESS_SUCCESS,
             25,
             2002,
             1,
             1,
             0,
             1,
             0,
             0,
             true);

  // Negative timezone carry

  // Minute => hour
  check_read("2001-02-03T04:14:00+00:15",
             EXESS_SUCCESS,
             25,
             2001,
             2,
             3,
             3,
             59,
             0,
             0,
             true);

  // Minute => hour => day
  check_read("2001-02-03T00:14:00+00:15",
             EXESS_SUCCESS,
             25,
             2001,
             2,
             2,
             23,
             59,
             0,
             0,
             true);

  // Minute => hour => day => month
  check_read("2001-02-01T00:14:00+00:15",
             EXESS_SUCCESS,
             25,
             2001,
             1,
             31,
             23,
             59,
             0,
             0,
             true);

  // Garbage

  check_read(
    "2004-04-12T13:00", EXESS_EXPECTED_COLON, 16, 0, 0, 0, 0, 0, 0, 0, false);

  check_read("2004-04-1213:20:00",
             EXESS_EXPECTED_TIME_SEP,
             10,
             0,
             0,
             0,
             0,
             0,
             0,
             0,
             false);

  check_read(
    "99-04-12T13:00", EXESS_EXPECTED_DIGIT, 2, 0, 0, 0, 0, 0, 0, 0, false);

  check_read(
    "2004-04-12", EXESS_EXPECTED_TIME_SEP, 10, 0, 0, 0, 0, 0, 0, 0, false);

  check_read("2004-04-12-05:00",
             EXESS_EXPECTED_TIME_SEP,
             10,
             0,
             0,
             0,
             0,
             0,
             0,
             0,
             false);
}

static void
check_write(const ExessDateTime value,
            const ExessStatus   expected_status,
            const size_t        buf_size,
            const char* const   expected_string)
{
  char buf[EXESS_MAX_DATETIME_LENGTH + 1] = {
    1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 16,
    17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32};

  assert(buf_size <= sizeof(buf));

  const ExessResult r = exess_write_datetime(value, buf_size, buf);
  assert(r.status == expected_status);
  assert(r.count == strlen(buf));
  assert(!strcmp(buf, expected_string));
  assert(r.status || exess_write_datetime(value, 0, NULL).count == r.count);
}

static void
test_write_datetime(void)
{
  check_write(local, EXESS_SUCCESS, 20, "2001-02-03T04:05:06");
  check_write(utc, EXESS_SUCCESS, 21, "2001-02-03T04:05:06Z");
  check_write(lowest, EXESS_SUCCESS, 22, "-32768-01-01T00:00:00");
  check_write(highest, EXESS_SUCCESS, 21, "32767-12-31T24:00:00");
  check_write(nano, EXESS_SUCCESS, 30, "2001-01-01T00:00:00.000000001");

  check_write(garbage1, EXESS_BAD_VALUE, 20, "");
  check_write(garbage2, EXESS_BAD_VALUE, 20, "");
  check_write(garbage3, EXESS_BAD_VALUE, 20, "");
  check_write(garbage4, EXESS_BAD_VALUE, 20, "");
  check_write(garbage5, EXESS_BAD_VALUE, 20, "");
  check_write(garbage6, EXESS_BAD_VALUE, 20, "");
  check_write(garbage7, EXESS_BAD_VALUE, 20, "");
  check_write(garbage8, EXESS_BAD_VALUE, 20, "");
  check_write(garbage9, EXESS_BAD_VALUE, 20, "");
  check_write(garbage10, EXESS_BAD_VALUE, 20, "");
  check_write(garbage11, EXESS_BAD_VALUE, 20, "");

  check_write(lowest, EXESS_NO_SPACE, 12, "");
  check_write(lowest, EXESS_NO_SPACE, 17, "");
  check_write(lowest, EXESS_NO_SPACE, 18, "");
  check_write(lowest, EXESS_NO_SPACE, 21, "");

  // Check that nothing is written when there isn't enough space
  char              c = 42;
  const ExessResult r = exess_write_datetime(highest, 0, &c);
  assert(c == 42);
  assert(r.status == EXESS_NO_SPACE);
  assert(r.count == 0);
}

int
main(void)
{
  test_add();
  test_calendar();
  test_read_datetime();
  test_write_datetime();

  return 0;
}
