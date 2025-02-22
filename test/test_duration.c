// Copyright 2011-2025 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#undef NDEBUG

#include "int_test_utils.h"
#include "num_test_utils.h"

#include <exess/exess.h>

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static const ExessDuration zero       = {0, 0, 0};
static const ExessDuration lowest     = {-INT32_MAX, -INT32_MAX, -999999999};
static const ExessDuration highest    = {INT32_MAX, INT32_MAX, 999999999};
static const ExessDuration year       = {12, 0, 0};
static const ExessDuration month      = {1, 0, 0};
static const ExessDuration day        = {0, 24 * 60 * 60, 0};
static const ExessDuration hour       = {0, 60 * 60, 0};
static const ExessDuration minute     = {0, 60, 0};
static const ExessDuration second     = {0, 1, 0};
static const ExessDuration nanosecond = {0, 0, 1};

static const ExessDuration n_year       = {-12, 0, 0};
static const ExessDuration n_month      = {-1, 0, 0};
static const ExessDuration n_day        = {0, -24 * 60 * 60, 0};
static const ExessDuration n_hour       = {0, -60 * 60, 0};
static const ExessDuration n_minute     = {0, -60, 0};
static const ExessDuration n_second     = {0, -1, 0};
static const ExessDuration n_nanosecond = {0, 0, -1};

static const ExessDuration garbage1 = {1, 1, -1};
static const ExessDuration garbage2 = {1, -1, 1};
static const ExessDuration garbage3 = {1, -1, -1};
static const ExessDuration garbage4 = {-1, 1, 1};
static const ExessDuration garbage5 = {-1, 1, -1};
static const ExessDuration garbage6 = {-1, -1, 1};
static const ExessDuration garbage7 = {INT32_MIN, 0, -999999999};
static const ExessDuration garbage8 = {0, INT32_MIN, -999999999};
static const ExessDuration garbage9 = {INT32_MIN, INT32_MIN, -999999999};

static void
check_read(const char* const string,
           const ExessStatus expected_status,
           const size_t      expected_count,
           const int32_t     expected_years,
           const int32_t     expected_months,
           const int32_t     expected_days,
           const int32_t     expected_hours,
           const int32_t     expected_minutes,
           const int32_t     expected_seconds,
           const int32_t     expected_nanoseconds,
           const bool        expected_is_negative)
{
  ExessDuration     value = {0, 0, 0};
  const ExessResult r     = exess_read_duration(&value, string);

  assert(r.status == expected_status);
  assert(r.count == expected_count);

  assert(value.months == (expected_is_negative ? -1 : 1) * 12 * expected_years +
                           expected_months);

  assert(value.seconds ==
         (expected_is_negative ? -1 : 1) *
           ((expected_seconds + (60 * expected_minutes) +
             (60 * 60 * expected_hours) + (24 * 60 * 60 * expected_days))));

  assert(value.nanoseconds == expected_nanoseconds);
}

static void
test_read_duration(void)
{
  // No input
  check_read("", EXESS_EXPECTED_DURATION, 0, 0, 0, 0, 0, 0, 0, 0, false);
  check_read(
    " \f\n\r\t\v", EXESS_EXPECTED_DURATION, 6, 0, 0, 0, 0, 0, 0, 0, false);

  // Good values

  check_read(
    "P2Y6M5DT12H35M30S", EXESS_SUCCESS, 17, 2, 6, 5, 12, 35, 30, 0, false);

  check_read("P1DT2H", EXESS_SUCCESS, 6, 0, 0, 1, 2, 0, 0, 0, false);
  check_read("P20M", EXESS_SUCCESS, 4, 0, 20, 0, 0, 0, 0, 0, false);
  check_read("PT20M", EXESS_SUCCESS, 5, 0, 0, 0, 0, 20, 0, 0, false);
  check_read("P0Y20M0D", EXESS_SUCCESS, 8, 0, 20, 0, 0, 0, 0, 0, false);
  check_read("P0Y", EXESS_SUCCESS, 3, 0, 0, 0, 0, 0, 0, 0, false);
  check_read("-P60D", EXESS_SUCCESS, 5, 0, 0, 60, 0, 0, 0, 0, true);
  check_read(
    "PT1M30.5S", EXESS_SUCCESS, 9, 0, 0, 0, 0, 1, 30, 500000000, false);

  // Leading and trailing whitespace
  check_read(" \f\n\r\t\vP1Y", EXESS_SUCCESS, 9, 1, 0, 0, 0, 0, 0, 0, false);
  check_read("P1MT2H \f\n\r\t\v", EXESS_SUCCESS, 6, 0, 1, 0, 2, 0, 0, 0, false);
  check_read(" \f\n\r\t\vP1Y", EXESS_SUCCESS, 9, 1, 0, 0, 0, 0, 0, 0, false);
  check_read("P1YT2H \f\n\r\t\v", EXESS_SUCCESS, 6, 1, 0, 0, 2, 0, 0, 0, false);

  // Non-canonical form
  check_read("P06D", EXESS_SUCCESS, 4, 0, 0, 6, 0, 0, 0, 0, false);
  check_read("PT7.0S", EXESS_SUCCESS, 6, 0, 0, 0, 0, 0, 7, 0, false);
  check_read(
    "P0Y0M01DT06H00M00S", EXESS_SUCCESS, 18, 0, 0, 1, 6, 0, 0, 0, false);

  // Out of range fields
  check_read(
    "P2147483647Y", EXESS_OUT_OF_RANGE, 12, 0, 0, 0, 0, 0, 0, 0, false);
  check_read(
    "P2147483647M", EXESS_OUT_OF_RANGE, 12, 0, 0, 0, 0, 0, 0, 0, false);
  check_read(
    "P2147483647D", EXESS_OUT_OF_RANGE, 12, 0, 0, 0, 0, 0, 0, 0, false);
  check_read(
    "PT2147483647H", EXESS_OUT_OF_RANGE, 13, 0, 0, 0, 0, 0, 0, 0, false);
  check_read(
    "PT2147483647M", EXESS_OUT_OF_RANGE, 13, 0, 0, 0, 0, 0, 0, 0, false);
  check_read(
    "PT2147483647S", EXESS_OUT_OF_RANGE, 13, 0, 0, 0, 0, 0, 0, 0, false);

  // Garbage
  check_read("P-20M", EXESS_EXPECTED_DIGIT, 1, 0, 0, 0, 0, 0, 0, 0, false);
  check_read("P20MT", EXESS_EXPECTED_DIGIT, 5, 0, 20, 0, 0, 0, 0, 0, false);
  check_read("P1DX", EXESS_EXPECTED_TIME_SEP, 3, 0, 0, 1, 0, 0, 0, 0, false);
  check_read("P1YM5D", EXESS_EXPECTED_DIGIT, 3, 1, 0, 0, 0, 0, 0, 0, false);
  check_read("P15.5Y", EXESS_EXPECTED_DATE_TAG, 3, 0, 0, 0, 0, 0, 0, 0, false);
  check_read("P1D2H", EXESS_EXPECTED_TIME_SEP, 3, 0, 0, 1, 0, 0, 0, 0, false);
  check_read("1Y2M", EXESS_EXPECTED_DURATION, 0, 0, 0, 0, 0, 0, 0, 0, false);
  check_read("P2M1Y", EXESS_BAD_ORDER, 5, 0, 2, 0, 0, 0, 0, 0, false);
  check_read("P2D1Y", EXESS_EXPECTED_TIME_SEP, 3, 0, 0, 2, 0, 0, 0, 0, false);
  check_read("P", EXESS_EXPECTED_DIGIT, 1, 0, 0, 0, 0, 0, 0, 0, false);
  check_read("PT15.5H", EXESS_EXPECTED_TIME_TAG, 6, 0, 0, 0, 0, 0, 0, 0, false);
  check_read("PT2M1H", EXESS_BAD_ORDER, 6, 0, 0, 0, 0, 2, 0, 0, false);
  check_read("PT2S1H", EXESS_BAD_ORDER, 6, 0, 0, 0, 0, 0, 2, 0, false);
  check_read("PT2S1M", EXESS_BAD_ORDER, 6, 0, 0, 0, 0, 0, 2, 0, false);
  check_read("PT15.S", EXESS_EXPECTED_DIGIT, 5, 0, 0, 0, 0, 0, 0, 0, false);
  check_read("P1Q", EXESS_EXPECTED_DATE_TAG, 2, 0, 0, 0, 0, 0, 0, 0, false);
  check_read("PT1Q", EXESS_EXPECTED_TIME_TAG, 3, 0, 0, 0, 0, 0, 0, 0, false);
  check_read("P-1Y", EXESS_EXPECTED_DIGIT, 1, 0, 0, 0, 0, 0, 0, 0, false);
  check_read("P-1M", EXESS_EXPECTED_DIGIT, 1, 0, 0, 0, 0, 0, 0, 0, false);
  check_read("P-1D", EXESS_EXPECTED_DIGIT, 1, 0, 0, 0, 0, 0, 0, 0, false);
  check_read("PT-1H", EXESS_EXPECTED_DIGIT, 2, 0, 0, 0, 0, 0, 0, 0, false);
  check_read("PT-1M", EXESS_EXPECTED_DIGIT, 2, 0, 0, 0, 0, 0, 0, 0, false);
  check_read("PT-1S", EXESS_EXPECTED_DIGIT, 2, 0, 0, 0, 0, 0, 0, 0, false);
  check_read(
    "P4294967296Y", EXESS_OUT_OF_RANGE, 12, 0, 0, 0, 0, 0, 0, 0, false);
  check_read(
    "P4294967296M", EXESS_OUT_OF_RANGE, 12, 0, 0, 0, 0, 0, 0, 0, false);
  check_read(
    "P4294967296D", EXESS_OUT_OF_RANGE, 12, 0, 0, 0, 0, 0, 0, 0, false);
  check_read(
    "PT4294967296H", EXESS_OUT_OF_RANGE, 13, 0, 0, 0, 0, 0, 0, 0, false);
  check_read(
    "PT4294967296M", EXESS_OUT_OF_RANGE, 13, 0, 0, 0, 0, 0, 0, 0, false);
  check_read("", EXESS_EXPECTED_DURATION, 0, 0, 0, 0, 0, 0, 0, 0, false);
}

static void
check_write(const ExessDuration value,
            const ExessStatus   expected_status,
            const size_t        buf_size,
            const char* const   expected_string)
{
  char buf[EXESS_MAX_DURATION_LENGTH + 1] = {42};

  assert(buf_size <= sizeof(buf));

  const ExessResult r = exess_write_duration(value, buf_size, buf);
  assert(r.status == expected_status);
  assert(r.count == strlen(buf));
  assert(!strcmp(buf, expected_string));
  assert(r.status || exess_write_duration(value, 0, NULL).count == r.count);
}

static void
test_write_duration(void)
{
  check_write(zero, EXESS_SUCCESS, 4, "P0Y");

  check_write(
    lowest, EXESS_SUCCESS, 39, "-P178956970Y7M24855DT3H14M7.999999999S");

  check_write(
    highest, EXESS_SUCCESS, 38, "P178956970Y7M24855DT3H14M7.999999999S");

  check_write(year, EXESS_SUCCESS, 4, "P1Y");
  check_write(month, EXESS_SUCCESS, 4, "P1M");
  check_write(day, EXESS_SUCCESS, 4, "P1D");
  check_write(hour, EXESS_SUCCESS, 5, "PT1H");
  check_write(minute, EXESS_SUCCESS, 5, "PT1M");
  check_write(second, EXESS_SUCCESS, 5, "PT1S");
  check_write(nanosecond, EXESS_SUCCESS, 15, "PT0.000000001S");

  check_write(n_year, EXESS_SUCCESS, 5, "-P1Y");
  check_write(n_month, EXESS_SUCCESS, 5, "-P1M");
  check_write(n_day, EXESS_SUCCESS, 5, "-P1D");
  check_write(n_hour, EXESS_SUCCESS, 6, "-PT1H");
  check_write(n_minute, EXESS_SUCCESS, 6, "-PT1M");
  check_write(n_second, EXESS_SUCCESS, 6, "-PT1S");
  check_write(n_nanosecond, EXESS_SUCCESS, 16, "-PT0.000000001S");

  check_write(garbage1, EXESS_BAD_VALUE, 41, "");
  check_write(garbage2, EXESS_BAD_VALUE, 41, "");
  check_write(garbage3, EXESS_BAD_VALUE, 41, "");
  check_write(garbage4, EXESS_BAD_VALUE, 41, "");
  check_write(garbage5, EXESS_BAD_VALUE, 41, "");
  check_write(garbage6, EXESS_BAD_VALUE, 41, "");
  check_write(garbage7, EXESS_OUT_OF_RANGE, 41, "");
  check_write(garbage8, EXESS_OUT_OF_RANGE, 41, "");
  check_write(garbage9, EXESS_OUT_OF_RANGE, 41, "");

  check_write(zero, EXESS_NO_SPACE, 3, "");
  check_write(lowest, EXESS_NO_SPACE, 24, "");
  check_write(highest, EXESS_NO_SPACE, 4, "");
  check_write(highest, EXESS_NO_SPACE, 10, "");
  check_write(highest, EXESS_NO_SPACE, 13, "");
  check_write(highest, EXESS_NO_SPACE, 16, "");
  check_write(highest, EXESS_NO_SPACE, 20, "");
  check_write(highest, EXESS_NO_SPACE, 23, "");
  check_write(year, EXESS_NO_SPACE, 3, "");
  check_write(month, EXESS_NO_SPACE, 3, "");
  check_write(day, EXESS_NO_SPACE, 3, "");
  check_write(hour, EXESS_NO_SPACE, 4, "");
  check_write(minute, EXESS_NO_SPACE, 4, "");
  check_write(second, EXESS_NO_SPACE, 4, "");

  // Check that nothing is written when there isn't enough space
  char              c = 42;
  const ExessResult r = exess_write_duration(zero, 0, &c);
  assert(c == 42);
  assert(r.status == EXESS_NO_SPACE);
  assert(r.count == 0);
}

static void
check_round_trip(const ExessDuration value)
{
  ExessDuration parsed_value                       = {0, 0, 0};
  char          buf[EXESS_MAX_DURATION_LENGTH + 1] = {0};

  assert(exess_write_duration(value, 0, NULL).count <=
         EXESS_MAX_DURATION_LENGTH);

  assert(!exess_write_duration(value, sizeof(buf), buf).status);
  assert(!exess_read_duration(&parsed_value, buf).status);
  assert(parsed_value.months == value.months);
  assert(parsed_value.seconds == value.seconds);
  assert(parsed_value.nanoseconds == value.nanoseconds);
}

static void
test_round_trip(const ExessNumTestOptions opts)
{
  fprintf(stderr, "Testing xsd:duration randomly with seed %u\n", opts.seed);

  const uint64_t n_tests =
    (opts.n_tests >= 4096U) ? (opts.n_tests / 16U) : 256U;

  uint32_t rng = opts.seed;
  for (size_t i = 0; i < n_tests; ++i) {
    rng = lcg32(rng);

    const int32_t months = (int32_t)rng;

    rng = lcg32(rng);

    const int32_t seconds = (months < 0 ? -1 : 1) * (int32_t)(rng % INT32_MAX);

    rng = lcg32(rng);

    const int32_t nanoseconds =
      (months < 0 ? -1 : 1) * (int32_t)(rng % 1000000000);

    const ExessDuration value = {months, seconds, nanoseconds};
    check_round_trip(value);

    print_num_test_progress(i, n_tests);
  }
}

int
main(int argc, char** argv)
{
  const ExessNumTestOptions opts =
    parse_num_test_options(argc, argv, 16384U, 0U, INT32_MAX);

  if (!opts.error) {
    test_read_duration();
    test_write_duration();
    test_round_trip(opts);
  }

  return (int)opts.error;
}
