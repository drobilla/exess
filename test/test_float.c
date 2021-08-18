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
           const float       expected_value,
           const size_t      expected_count)
{
  float             value = NAN;
  const ExessResult r     = exess_read_float(&value, string);

  assert(r.status == expected_status);
  assert(r.count == expected_count);
  assert(float_matches(value, expected_value));
}

static void
test_read_float(void)
{
  // Limits
  check_read("-3.40282347E38", EXESS_SUCCESS, -FLT_MAX, 14);
  check_read("-1.17549435E-38", EXESS_SUCCESS, -FLT_MIN, 15);
  check_read("1.17549435E-38", EXESS_SUCCESS, FLT_MIN, 14);
  check_read("3.40282347E38", EXESS_SUCCESS, FLT_MAX, 13);

  // Special values
  check_read("NaN", EXESS_SUCCESS, NAN, 3);
  check_read("-INF", EXESS_SUCCESS, -INFINITY, 4);
  check_read("-0.0E0", EXESS_SUCCESS, -0.0f, 6);
  check_read("0.0E0", EXESS_SUCCESS, 0.0f, 5);
  check_read("+0.0E0", EXESS_SUCCESS, 0.0f, 6);
  check_read("INF", EXESS_SUCCESS, INFINITY, 3);
  check_read("+INF", EXESS_SUCCESS, INFINITY, 4);

  // Various normal cases
  check_read("-1.0E0", EXESS_SUCCESS, -1.0f, 6);
  check_read("1.0E0", EXESS_SUCCESS, +1.0f, 5);
  check_read("5.0E0", EXESS_SUCCESS, 5.0f, 5);
  check_read("5.0E1", EXESS_SUCCESS, 50.0f, 5);
  check_read("5.0E9", EXESS_SUCCESS, 5000000000.0f, 5);
  check_read("-5.0E-1", EXESS_SUCCESS, -0.5f, 7);
  check_read("5.0E-1", EXESS_SUCCESS, 0.5f, 6);
  check_read("6.25E-2", EXESS_SUCCESS, 0.0625f, 7);
  check_read("7.8125E-3", EXESS_SUCCESS, 0.0078125f, 9);

  // No exponent
  check_read("1", EXESS_SUCCESS, 1.0f, 1);
  check_read("2.3", EXESS_SUCCESS, 2.3f, 3);
  check_read("-4.5", EXESS_SUCCESS, -4.5f, 4);

  // Trailing garbage
  check_read("1.2.", EXESS_SUCCESS, 1.2f, 3);

  // Garbage
  check_read("true", EXESS_EXPECTED_DIGIT, NAN, 0);
  check_read("+true", EXESS_EXPECTED_DIGIT, NAN, 1);
  check_read("-false", EXESS_EXPECTED_DIGIT, NAN, 1);
  check_read("1.0eX", EXESS_EXPECTED_DIGIT, NAN, 4);
  check_read("1.0EX", EXESS_EXPECTED_DIGIT, NAN, 4);
}

static void
test_float_string_length(void)
{
  // Limits
  assert(exess_write_float(FLT_MIN, 0, NULL).count == 14);
  assert(exess_write_float(FLT_MAX, 0, NULL).count == 13);

  // Special values
  assert(exess_write_float((float)NAN, 0, NULL).count == 3);
  assert(exess_write_float(-1.0f, 0, NULL).count == 6);
  assert(exess_write_float(-0.0f, 0, NULL).count == 6);
  assert(exess_write_float(0.0f, 0, NULL).count == 5);
  assert(exess_write_float(1.0f, 0, NULL).count == 5);
  assert(exess_write_float((float)INFINITY, 0, NULL).count == 3);
  assert(exess_write_float((float)-INFINITY, 0, NULL).count == 4);
}

/// Check that `str` is a canonical xsd:float string
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
check_write(const float       value,
            const ExessStatus expected_status,
            const size_t      buf_size,
            const char* const expected_string)
{
  char buf[EXESS_MAX_FLOAT_LENGTH + 1] = {
    1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};

  assert(buf_size <= sizeof(buf));

  const ExessResult r = exess_write_float(value, buf_size, buf);
  assert(r.status == expected_status);
  if (expected_string) {
    assert(r.count == strlen(buf));
    assert(!expected_string || !strcmp(buf, expected_string));
    assert(r.status || exess_write_float(value, 0, NULL).count == r.count);
    check_canonical(buf);
  } else {
    assert(r.count == 0);
  }
}

static void
test_write_float(void)
{
  check_write(NAN, EXESS_SUCCESS, 4, "NaN");
  check_write(-INFINITY, EXESS_SUCCESS, 5, "-INF");
  check_write(FLT_MIN, EXESS_SUCCESS, 15, "1.17549435E-38");
  check_write(-0.0f, EXESS_SUCCESS, 7, "-0.0E0");
  check_write(0.0f, EXESS_SUCCESS, 6, "0.0E0");
  check_write(100.25f, EXESS_SUCCESS, 9, "1.0025E2");
  check_write(FLT_MAX, EXESS_SUCCESS, 14, "3.40282346E38");
  check_write(INFINITY, EXESS_SUCCESS, 4, "INF");

  check_write(NAN, EXESS_NO_SPACE, 3, NULL);
  check_write(-INFINITY, EXESS_NO_SPACE, 4, NULL);
  check_write(FLT_MIN, EXESS_NO_SPACE, 13, NULL);
  check_write(-1.0f, EXESS_NO_SPACE, 2, NULL);
  check_write(-0.0f, EXESS_NO_SPACE, 6, NULL);
  check_write(0.0f, EXESS_NO_SPACE, 5, NULL);
  check_write(100.25f, EXESS_NO_SPACE, 5, NULL);
  check_write(100.25f, EXESS_NO_SPACE, 8, NULL);
  check_write(FLT_MAX, EXESS_NO_SPACE, 13, NULL);
  check_write(INFINITY, EXESS_NO_SPACE, 3, NULL);
}

static void
check_round_trip(const float value)
{
  float parsed_value                    = 0.0f;
  char  buf[EXESS_MAX_FLOAT_LENGTH + 1] = {
    1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};

  assert(!exess_write_float(value, sizeof(buf), buf).status);
  assert(!exess_read_float(&parsed_value, buf).status);
  assert(float_matches(parsed_value, value));
}

static void
test_round_trip(const ExessNumTestOptions opts)
{
  check_round_trip(NAN);
  check_round_trip(FLT_MIN);
  check_round_trip(-0.0f);
  check_round_trip(0.0f);
  check_round_trip(FLT_MAX);

  if (opts.exhaustive) {
    fprintf(stderr, "Testing xsd:float exhaustively\n");

    for (int64_t i = 0; i <= UINT32_MAX; ++i) {
      const float value = float_from_rep((uint32_t)i);

      check_round_trip(value);
      print_num_test_progress((uint64_t)(i - (int64_t)INT32_MIN), UINT32_MAX);
    }
  } else {
    fprintf(stderr, "Testing xsd:float randomly with seed %u\n", opts.seed);

    uint32_t rep = opts.seed;
    for (uint64_t i = 0; i < opts.n_tests; ++i) {
      rep = lcg32(rep);

      const float value = float_from_rep(rep);

      check_round_trip(nextafterf(value, -INFINITY));
      check_round_trip(value);
      check_round_trip(nextafterf(value, INFINITY));

      print_num_test_progress(i, opts.n_tests);
    }
  }
}

int
main(int argc, char** argv)
{
  const ExessNumTestOptions opts = parse_num_test_options(argc, argv);
  if (opts.error) {
    return 1;
  }

  test_read_float();
  test_float_string_length();
  test_write_float();
  test_round_trip(opts);

  return 0;
}
