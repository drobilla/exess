// Copyright 2011-2021 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#undef NDEBUG

#include "float_test_data.h"
#include "int_test_data.h"
#include "num_test_utils.h"
#include "string_utils.h"

#include "exess/exess.h"

#include <assert.h>
#include <float.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static void
check_read(const char* const string,
           const ExessStatus expected_status,
           const double      expected_value,
           const size_t      expected_count)
{
  double            value = 0;
  const ExessResult r     = exess_read_decimal(&value, string);

  assert(r.status == expected_status);
  assert(r.count == expected_count);
  assert(double_matches(value, expected_value));
}

static void
test_read_decimal(void)
{
  // No value
  check_read("", EXESS_EXPECTED_DIGIT, (double)NAN, 0);
  check_read(" \f\n\r\t\v", EXESS_EXPECTED_DIGIT, (double)NAN, 6);

  // Basic values
  check_read("1.2", EXESS_SUCCESS, 1.2, 3);
  check_read("0.01", EXESS_SUCCESS, 0.01, 4);
  check_read("10.0", EXESS_SUCCESS, 10.0, 4);

  // Non-canonical form
  check_read(" \f\n\r\t\v42.24 ", EXESS_SUCCESS, 42.24, 11);
  check_read("12.", EXESS_SUCCESS, 12., 3);
  check_read(".34", EXESS_SUCCESS, 0.34, 3);
  check_read("+.56", EXESS_SUCCESS, 0.56, 4);
  check_read("-.78", EXESS_SUCCESS, -0.78, 4);

  // Limits
  check_read(
    "0."
    "00000000000000000000000000000000000000000000000000000000000000000000000000"
    "00000000000000000000000000000000000000000000000000000000000000000000000000"
    "00000000000000000000000000000000000000000000000000000000000000000000000000"
    "00000000000000000000000000000000000000000000000000000000000000000000000000"
    "0000000000022250738585072014",
    EXESS_SUCCESS,
    DBL_MIN,
    326);

  check_read("1797693134862315700000000000000000000000000000000000000000000000"
             "0000000000000000000000000000000000000000000000000000000000000000"
             "0000000000000000000000000000000000000000000000000000000000000000"
             "0000000000000000000000000000000000000000000000000000000000000000"
             "00000000000000000000000000000000000000000000000000000.0",
             EXESS_SUCCESS,
             DBL_MAX,
             311);

  // Superfluous digits
  check_read("12345678901234567890", EXESS_SUCCESS, 12345678901234568000.0, 20);
  check_read("1.2345678901234567890", EXESS_SUCCESS, 1.2345678901234568, 21);

  // Special values
  check_read("-0.0E0", EXESS_EXPECTED_END, -0.0, 4);
  check_read("0.0E0", EXESS_EXPECTED_END, 0.0, 3);
  check_read("+0.0E0", EXESS_EXPECTED_END, 0.0, 4);

  // No exponent
  check_read("1", EXESS_SUCCESS, 1.0, 1);
  check_read("2.3", EXESS_SUCCESS, 2.3, 3);
  check_read("-4.5", EXESS_SUCCESS, -4.5, 4);

  // Garbage
  check_read("NaN", EXESS_EXPECTED_DIGIT, (double)NAN, 0);
  check_read("INF", EXESS_EXPECTED_DIGIT, (double)NAN, 0);
  check_read("-INF", EXESS_EXPECTED_DIGIT, (double)NAN, 1);
  check_read("true", EXESS_EXPECTED_DIGIT, (double)NAN, 0);
  check_read("+true", EXESS_EXPECTED_DIGIT, (double)NAN, 1);
  check_read("-false", EXESS_EXPECTED_DIGIT, (double)NAN, 1);
}

static void
test_decimal_string_length(void)
{
  // Basic values
  assert(exess_write_decimal(-1.0, 0, NULL).count == 4);
  assert(exess_write_decimal(-0.0, 0, NULL).count == 4);
  assert(exess_write_decimal(0.0, 0, NULL).count == 3);
  assert(exess_write_decimal(1.0, 0, NULL).count == 3);

  // Limits
  assert(exess_write_decimal(DBL_MIN, 0, NULL).count == 326);
  assert(exess_write_decimal(DBL_MAX, 0, NULL).count == 311);

  // Special values
  assert(exess_write_decimal((double)NAN, 0, NULL).count == 0);
  assert(exess_write_decimal(-0.0, 0, NULL).count == 4);
  assert(exess_write_decimal(0.0, 0, NULL).count == 3);
  assert(exess_write_decimal((double)INFINITY, 0, NULL).count == 0);
  assert(exess_write_decimal((double)-INFINITY, 0, NULL).count == 0);
}

/// Check that `str` is a canonical xsd:double string
static void
check_canonical(const char* const str)
{
  assert(strlen(str) > 2); // Shortest possible is something like 1.2
  assert(str[0] == '-' || is_digit(str[0]));

  const int first_digit = str[0] == '-' ? 1 : 0;

  for (const char* s = str + first_digit; *s; ++s) {
    assert(*s == '.' || is_digit(*s));
  }
}

static void
check_write(const double      value,
            const ExessStatus expected_status,
            const size_t      buf_size,
            const char* const expected_string)
{
  char buf[EXESS_MAX_DECIMAL_LENGTH + 1] = {42};

  assert(buf_size <= sizeof(buf));

  const ExessResult r = exess_write_decimal(value, buf_size, buf);
  assert(r.status == expected_status);
  if (expected_string) {
    assert(r.count == strlen(buf));
    assert(!strcmp(buf, expected_string));
    assert(r.status || exess_write_decimal(value, 0, NULL).count == r.count);

    if (expected_string[0]) {
      check_canonical(buf);
    }
  }
}

static void
test_write_decimal(void)
{
  check_write((double)NAN, EXESS_BAD_VALUE, 4, "");
  check_write((double)-INFINITY, EXESS_BAD_VALUE, 5, "");

  check_write(
    DBL_MIN,
    EXESS_SUCCESS,
    327,
    "0."
    "00000000000000000000000000000000000000000000000000000000000000000000000000"
    "00000000000000000000000000000000000000000000000000000000000000000000000000"
    "00000000000000000000000000000000000000000000000000000000000000000000000000"
    "00000000000000000000000000000000000000000000000000000000000000000000000000"
    "0000000000022250738585072014");

  check_write(-1.2, EXESS_SUCCESS, 5, "-1.2");
  check_write(-0.0, EXESS_SUCCESS, 5, "-0.0");
  check_write(0.0, EXESS_SUCCESS, 4, "0.0");
  check_write(1.2, EXESS_SUCCESS, 4, "1.2");

  check_write(DBL_MAX,
              EXESS_SUCCESS,
              312,
              "1797693134862315700000000000000000000000000000000000000000000000"
              "0000000000000000000000000000000000000000000000000000000000000000"
              "0000000000000000000000000000000000000000000000000000000000000000"
              "0000000000000000000000000000000000000000000000000000000000000000"
              "00000000000000000000000000000000000000000000000000000.0");

  check_write((double)INFINITY, EXESS_BAD_VALUE, 4, "");

  check_write(DBL_MIN, EXESS_NO_SPACE, 326, "");
  check_write(-1.2, EXESS_NO_SPACE, 4, "");
  check_write(-0.0, EXESS_NO_SPACE, 4, "");
  check_write(0.0, EXESS_NO_SPACE, 3, "");
  check_write(1.2, EXESS_NO_SPACE, 3, "");
  check_write(DBL_MAX, EXESS_NO_SPACE, 311, "");

  check_write(-1.0, EXESS_NO_SPACE, 1, "");
  check_write(-1.0, EXESS_NO_SPACE, 0, NULL);
}

static void
check_round_trip(const double value)
{
  double parsed_value                      = 0.0;
  char   buf[EXESS_MAX_DECIMAL_LENGTH + 1] = {42};

  assert(!exess_write_decimal(value, sizeof(buf), buf).status);
  assert(!exess_read_decimal(&parsed_value, buf).status);
  assert(double_matches(parsed_value, value));
}

static void
test_round_trip(const ExessNumTestOptions opts)
{
  check_round_trip(DBL_MIN);
  check_round_trip(-0.0);
  check_round_trip(0.0);
  check_round_trip(DBL_MAX);

  fprintf(stderr, "Testing xsd:double randomly with seed %u\n", opts.seed);

  uint32_t rep = opts.seed;
  for (uint64_t i = 0; i < opts.n_tests; ++i) {
    rep = lcg32(rep);

    const double value = double_from_rep(rep);

    check_round_trip(value);
    print_num_test_progress(i, opts.n_tests);
  }
}

int
main(int argc, char** argv)
{
  const ExessNumTestOptions opts = parse_num_test_options(argc, argv);

  if (!opts.error) {
    test_read_decimal();
    test_decimal_string_length();
    test_write_decimal();
    test_round_trip(opts);
  }

  return (int)opts.error;
}
