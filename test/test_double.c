// Copyright 2011-2021 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#undef NDEBUG

#include "float_test_data.h"
#include "int_test_data.h"
#include "num_test_utils.h"
#include "string_utils.h"
#include "warnings.h"

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
  double            value = (double)NAN;
  const ExessResult r     = exess_read_double(&value, string);

  assert(r.status == expected_status);
  assert(r.count == expected_count);
  assert(double_matches(value, expected_value));

  EXESS_DISABLE_CONVERSION_WARNINGS
  assert(isnan(value) || !double_matches(value, (double)NAN));
}

static void
test_read_double(void)
{
  // Limits
  check_read("-1.7976931348623157E308", EXESS_SUCCESS, -DBL_MAX, 23);
  check_read("-2.2250738585072014E-308", EXESS_SUCCESS, -DBL_MIN, 24);
  check_read("2.2250738585072014E-308", EXESS_SUCCESS, DBL_MIN, 23);
  check_read("1.7976931348623157E308", EXESS_SUCCESS, DBL_MAX, 22);

  // Beyond limits
  check_read("1e-326", EXESS_SUCCESS, 0.0, 6);
  check_read("12345678901234567123", EXESS_SUCCESS, 12345678901234567000.0, 20);
  check_read("1e309", EXESS_SUCCESS, (double)INFINITY, 5);

  // Non-canonical form
  check_read("+1E3", EXESS_SUCCESS, 1e3, 4);
  check_read("1E+3", EXESS_SUCCESS, 1e3, 4);
  check_read("+1.5E3", EXESS_SUCCESS, 1.5e3, 6);
  check_read(".5E3", EXESS_SUCCESS, 0.5e3, 4);
  check_read("+.5E3", EXESS_SUCCESS, 0.5e3, 5);
  check_read("-.5E3", EXESS_SUCCESS, -0.5e3, 5);
  check_read("1.E3", EXESS_SUCCESS, 1e3, 4);
  check_read("+1.E3", EXESS_SUCCESS, 1e3, 5);
  check_read("-1.E3", EXESS_SUCCESS, -1e3, 5);

  // Special values
  check_read("NaN", EXESS_SUCCESS, (double)NAN, 3);
  check_read("-INF", EXESS_SUCCESS, (double)-INFINITY, 4);
  check_read("-0.0E0", EXESS_SUCCESS, -0.0, 6);
  check_read("0.0E0", EXESS_SUCCESS, 0.0, 5);
  check_read("+0.0E0", EXESS_SUCCESS, 0.0, 6);
  check_read("INF", EXESS_SUCCESS, (double)INFINITY, 3);
  check_read("+INF", EXESS_SUCCESS, (double)INFINITY, 4);

  // No exponent
  check_read("1", EXESS_SUCCESS, 1.0, 1);
  check_read("2.3", EXESS_SUCCESS, 2.3, 3);
  check_read("-4.5", EXESS_SUCCESS, -4.5, 4);

  // Leading whitespace
  check_read(" \f\n\r\t\v1.2", EXESS_SUCCESS, 1.2, 9);

  // Garbage
  check_read("true", EXESS_EXPECTED_DIGIT, (double)NAN, 0);
  check_read("+true", EXESS_EXPECTED_DIGIT, (double)NAN, 1);
  check_read("-false", EXESS_EXPECTED_DIGIT, (double)NAN, 1);
  check_read("1.0eX", EXESS_EXPECTED_DIGIT, (double)NAN, 4);
  check_read("1.0EX", EXESS_EXPECTED_DIGIT, (double)NAN, 4);
}

/// Check that `str` is a canonical xsd:double string
static void
check_canonical(const char* const str)
{
  if (!strcmp(str, "NaN") || !strcmp(str, "-INF") || !strcmp(str, "INF")) {
    return;
  }

  assert(strlen(str) > 4); // Shortest possible is something like 1.2E3
  assert(str[0] == '-' || is_digit(str[0]));

  const int first_digit = str[0] == '-' ? 1 : 0;
  assert(is_digit(str[first_digit]));
  assert(str[first_digit + 1] == '.');
  assert(is_digit(str[first_digit + 2]));

  const char* const e = strchr(str, 'E');
  assert(e);
  assert(*e == 'E');
  assert(*(e + 1) == '-' || is_digit(*(e + 1)));
}

static void
check_write(const double      value,
            const ExessStatus expected_status,
            const size_t      buf_size,
            const char* const expected_string)
{
  char buf[EXESS_MAX_DOUBLE_LENGTH + 1] = {
    1,  2,  3,  4,  5,  6,  7,  8,  9,  10, //
    11, 12, 13, 14, 15, 16, 17, 18, 19, 20, //
    21, 22, 23};

  assert(buf_size <= sizeof(buf));

  const ExessResult r = exess_write_double(value, buf_size, buf);
  assert(r.status == expected_status);
  assert(r.count == strlen(buf));
  assert(!strcmp(buf, expected_string));
  assert((r.status && r.status != EXESS_NO_SPACE) ||
         exess_write_double(value, 0, NULL).count == r.count);

  if (!r.status) {
    check_canonical(buf);
  }
}

static void
test_write_double(void)
{
  check_write((double)NAN, EXESS_SUCCESS, 4, "NaN");
  check_write(DBL_MIN, EXESS_SUCCESS, 24, "2.2250738585072014E-308");
  check_write(-0.0, EXESS_SUCCESS, 7, "-0.0E0");
  check_write(0.0, EXESS_SUCCESS, 6, "0.0E0");
  check_write(DBL_MAX, EXESS_SUCCESS, 23, "1.7976931348623157E308");
}

static void
check_round_trip(const double value)
{
  double parsed_value = 0.0;

  char buf[EXESS_MAX_DOUBLE_LENGTH + 1] = {
    1,  2,  3,  4,  5,  6,  7,  8,  9,  10, //
    11, 12, 13, 14, 15, 16, 17, 18, 19, 20, //
    21, 22, 23};

  assert(!exess_write_double(value, sizeof(buf), buf).status);
  assert(!exess_read_double(&parsed_value, buf).status);
  assert(double_matches(parsed_value, value));
}

static void
test_round_trip(const ExessNumTestOptions opts)
{
  check_round_trip((double)NAN);
  check_round_trip(-(double)INFINITY);
  check_round_trip(DBL_MIN);
  check_round_trip(-0.0);
  check_round_trip(0.0);
  check_round_trip(DBL_MAX);
  check_round_trip((double)INFINITY);

  check_round_trip(5.0);
  check_round_trip(50.0);
  check_round_trip(500000000000000000000.0);
  check_round_trip(-0.5);
  check_round_trip(0.5);
  check_round_trip(0.05);
  check_round_trip(0.005);
  check_round_trip(0.00000000000000000005);

  // Normal limits
  check_round_trip(nextafter(DBL_MIN, (double)INFINITY));
  check_round_trip(nextafter(DBL_EPSILON, (double)INFINITY));
  check_round_trip(nextafter(DBL_MAX, -(double)INFINITY));

  // Subnormals
  check_round_trip(nextafter(0.0, 1.0));
  check_round_trip(nextafter(nextafter(0.0, 1.0), 2.0));
  check_round_trip(nextafter(0.0, -1.0));
  check_round_trip(nextafter(nextafter(0.0, -1.0), -2.0));

  // Various tricky cases
  check_round_trip(1e23);
  check_round_trip(6.02951420360127e-309);
  check_round_trip(9.17857104364115e+288);
  check_round_trip(2.68248422823759e+22);

  // Powers of two (where the lower boundary is closer)
  for (int i = -1023; i <= 1023; ++i) {
    check_round_trip(pow(2, i));
  }

  fprintf(stderr, "Testing xsd:double randomly with seed %u\n", opts.seed);

  uint64_t rep = opts.seed;
  for (uint64_t i = 0; i < opts.n_tests; ++i) {
    rep = lcg64(rep);

    const double value = double_from_rep(rep);

    check_round_trip(nextafter(value, -(double)INFINITY));
    check_round_trip(value);
    check_round_trip(nextafter(value, (double)INFINITY));

    print_num_test_progress(i, opts.n_tests);
  }
}

int
main(int argc, char** argv)
{
  const ExessNumTestOptions opts =
    parse_num_test_options(argc, argv, 0, UINT32_MAX);

  if (!opts.error) {
    test_read_double();
    test_write_double();
    test_round_trip(opts);
  }

  return (int)opts.error;
}
