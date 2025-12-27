// Copyright 2011-2025 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#undef NDEBUG

#include "decimal_test_utils.h"
#include "double_test_utils.h"
#include "int_test_utils.h"
#include "num_test_utils.h"
#include "write_test_utils.h"

#include <exess/exess.h>

#include <assert.h>
#include <float.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define VALUE_SIZE 22U
#define STRING_SIZE 327U

static void
check_read(const char* const string,
           const ExessStatus expected_status,
           const double      expected_number,
           const size_t      expected_count)
{
  EXESS_ALIGN uint8_t value[VALUE_SIZE]          = {0};
  EXESS_ALIGN uint8_t expected_value[VALUE_SIZE] = {0};

  const size_t expected_value_size =
    set_decimal_value(sizeof(expected_value), expected_value, expected_number);

  const ExessVariableResult r =
    exess_read_decimal(string, sizeof(value), value);

  assert(r.status == expected_status);
  assert(r.read_count == expected_count);
  assert(r.write_count == expected_value_size);
  assert(!memcmp(expected_value, value, r.write_count));
}

static void
test_read_decimal(void)
{
  // No value
  check_read("", EXESS_EXPECTED_DIGIT, (double)NAN, 0);
  check_read("\t\n\r ", EXESS_EXPECTED_DIGIT, (double)NAN, 4);

  // Basic values
  check_read("-100.25", EXESS_SUCCESS, -100.25, 7);
  check_read("1.2", EXESS_SUCCESS, 1.2, 3);
  check_read("0.01", EXESS_SUCCESS, 0.01, 4);
  check_read("10.0", EXESS_SUCCESS, 10.0, 4);

  // Non-canonical form
  check_read("\t\n\r 42.24 ", EXESS_SUCCESS, 42.24, 9);
  check_read("12.", EXESS_SUCCESS, 12., 3);
  check_read(".34", EXESS_SUCCESS, 0.34, 3);
  check_read("+.56", EXESS_SUCCESS, 0.56, 4);
  check_read("-.78", EXESS_SUCCESS, -0.78, 4);
  check_read("+00.10", EXESS_SUCCESS, 0.1, 6);

  // Double limits

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
  check_read("12345678901234567890", EXESS_LOSS, 12345678901234568000.0, 20);
  check_read("1.2345678901234567890", EXESS_LOSS, 1.2345678901234568, 21);

  // Special values
  check_read("-0.0E0", EXESS_SUCCESS, -0.0, 4);
  check_read("0.0E0", EXESS_SUCCESS, 0.0, 3);
  check_read("+0.0E0", EXESS_SUCCESS, 0.0, 4);

  // No exponent
  check_read("1", EXESS_SUCCESS, 1.0, 1);
  check_read("2.3", EXESS_SUCCESS, 2.3, 3);
  check_read("-4.5", EXESS_SUCCESS, -4.5, 4);

  // Trailing dot
  check_read("0.0.1", EXESS_SUCCESS, 0.0, 3);
  check_read("0.1.2", EXESS_SUCCESS, 0.1, 3);
  check_read("1.2.3", EXESS_SUCCESS, 1.2, 3);

  // Garbage
  check_read("NaN", EXESS_EXPECTED_DIGIT, (double)NAN, 0);
  check_read("INF", EXESS_EXPECTED_DIGIT, (double)NAN, 0);
  check_read("-INF", EXESS_EXPECTED_DIGIT, (double)NAN, 1);
  check_read("true", EXESS_EXPECTED_DIGIT, (double)NAN, 0);
  check_read("+true", EXESS_EXPECTED_DIGIT, (double)NAN, 1);
  check_read("-false", EXESS_EXPECTED_DIGIT, (double)NAN, 1);

  // Insufficient output space
  EXESS_ALIGN char value[VALUE_SIZE] = {0};
  for (size_t i = 0U; i < 5; ++i) {
    assert(exess_read_decimal("1", i, value).status == EXESS_NO_SPACE);
  }
}

/// Check that `str` is a canonical xsd:decimal string
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
check_write(const double      number,
            const ExessStatus expected_status,
            const size_t      buf_size,
            const char* const expected_string)
{
  EXESS_ALIGN char value[VALUE_SIZE] = {0};
  set_decimal_value(sizeof(value), value, number);

  char buf[STRING_SIZE] = {42};
  init_out_buf(sizeof(buf), buf);

  assert(buf_size <= sizeof(buf));

  const ExessResult r =
    exess_write_decimal(sizeof(value), value, buf_size, buf);
  assert(r.status == expected_status);
  if (!r.status) {
    assert(expected_string);
    assert(r.count == strlen(buf));
    assert(buf[0]);
    assert(!strcmp(buf, expected_string));
    check_canonical(buf);

    assert(exess_write_decimal(sizeof(value), value, 0, NULL).count == r.count);

    for (size_t space = 0; space < buf_size; ++space) {
      buf[0] = 1;

      const ExessResult s =
        exess_write_decimal(sizeof(value), value, space, buf);

      assert(s.status == EXESS_NO_SPACE);
      assert(s.count == r.count);
      assert(buf[0] == (space ? 0 : 1));
    }
  } else {
    assert(!r.count);
    assert(!buf[0]);
    assert(!exess_write_decimal(sizeof(value), value, 0, NULL).count);
  }
}

static void
test_write_decimal(void)
{
  // Numbers
  check_write(-100.25, EXESS_SUCCESS, 8, "-100.25");
  check_write(-0.0, EXESS_SUCCESS, 5, "-0.0");
  check_write(0.0, EXESS_SUCCESS, 4, "0.0");
  check_write(10.25, EXESS_SUCCESS, 6, "10.25");

  // Limits

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

  check_write(DBL_MAX,
              EXESS_SUCCESS,
              312,
              "1797693134862315700000000000000000000000000000000000000000000000"
              "0000000000000000000000000000000000000000000000000000000000000000"
              "0000000000000000000000000000000000000000000000000000000000000000"
              "0000000000000000000000000000000000000000000000000000000000000000"
              "00000000000000000000000000000000000000000000000000000.0");

  // Bad values
  check_write((double)NAN, EXESS_BAD_VALUE, 4, "");
  check_write((double)-INFINITY, EXESS_BAD_VALUE, 5, "");
  check_write((double)INFINITY, EXESS_BAD_VALUE, 4, "");

  // Bad value size
  EXESS_ALIGN char value[VALUE_SIZE] = {0};
  char             buf[STRING_SIZE]  = {0};
  set_decimal_value(sizeof(value), value, 2.0);
  assert(exess_write_decimal(1, value, sizeof(buf), buf).status ==
         EXESS_BAD_VALUE);
}

static void
check_round_trip(const double number)
{
  EXESS_ALIGN uint8_t value[VALUE_SIZE]        = {0};
  EXESS_ALIGN uint8_t parsed_value[VALUE_SIZE] = {0};
  char                buf[STRING_SIZE]         = {42};

  set_decimal_value(sizeof(value), value, number);
  init_out_buf(sizeof(buf), buf);

  const ExessResult w =
    exess_write_decimal(sizeof(value), value, sizeof(buf), buf);

  const ExessVariableResult r =
    exess_read_decimal(buf, sizeof(parsed_value), parsed_value);

  assert(!w.status);
  assert(r.status <= EXESS_LOSS);
  assert(r.read_count == w.count);
  assert(!memcmp(parsed_value, value, r.write_count));

  double parsed_number = (double)NAN;
  assert(!exess_coerce_value(0U,
                             EXESS_DECIMAL,
                             sizeof(parsed_value),
                             &parsed_value,
                             EXESS_DOUBLE,
                             sizeof(parsed_number),
                             &parsed_number)
            .status);

  assert(double_matches(parsed_number, number));
}

static void
test_round_trip(const ExessNumTestOptions opts)
{
  check_round_trip(DBL_MIN);
  check_round_trip(-0.0);
  check_round_trip(0.0);
  check_round_trip(DBL_MAX);

  fprintf(stderr, "Testing xsd:decimal randomly with seed %u\n", opts.seed);

  uint64_t state = opts.seed;
  for (uint64_t i = 0; i < opts.n_tests; ++i) {
    state = lcg64(state);

    const double value = random_double(state);

    check_round_trip(value);
    print_num_test_progress(i, opts.n_tests);
  }
}

int
main(int argc, char** argv)
{
  const ExessNumTestOptions opts =
    parse_num_test_options(argc, argv, 4096U, 0U, UINT32_MAX);

  if (!opts.error) {
    test_read_decimal();
    test_write_decimal();
    test_round_trip(opts);
  }

  return (int)opts.error;
}
