// Copyright 2011-2025 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#undef NDEBUG

#include "decimal_test_utils.h"
#include "int_test_utils.h"
#include "integer_test_utils.h"
#include "num_test_utils.h"
#include "write_test_utils.h"

#include <exess/exess.h>

#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define VALUE_SIZE 28U
#define STRING_SIZE 28U

static void
check_read(const char* const string,
           const ExessStatus expected_status,
           const int64_t     expected_number,
           const size_t      expected_count)
{
  EXESS_ALIGN uint8_t value[VALUE_SIZE]          = {0};
  EXESS_ALIGN uint8_t expected_value[VALUE_SIZE] = {0};
  set_integer_value(sizeof(expected_value), expected_value, expected_number);

  const ExessVariableResult r =
    exess_read_integer(string, sizeof(value), value);

  assert(r.status == expected_status);
  assert(r.read_count == expected_count);
  assert(expected_status || !memcmp(expected_value, value, r.write_count));
}

static void
test_read_integer(void)
{
  // No value
  check_read("", EXESS_EXPECTED_DIGIT, 0, 0);
  check_read("\t\n\r ", EXESS_EXPECTED_DIGIT, 0, 4);

  // Basic values
  check_read("-34000", EXESS_SUCCESS, -34000, 6);
  check_read("-1200", EXESS_SUCCESS, -1200, 5);
  check_read("0", EXESS_SUCCESS, 0, 1);
  check_read("1", EXESS_SUCCESS, 1, 1);
  check_read("1200", EXESS_SUCCESS, 1200, 4);
  check_read("34000", EXESS_SUCCESS, 34000, 5);

  // Non-canonical form
  check_read("\t\n\r 42 ", EXESS_SUCCESS, 42, 6);
  check_read("+0", EXESS_SUCCESS, 0, 2);
  check_read("-0", EXESS_SUCCESS, 0, 2);
  check_read("012", EXESS_SUCCESS, 12, 3);
  check_read("-034", EXESS_SUCCESS, -34, 4);
  check_read("+56", EXESS_SUCCESS, 56, 3);
  check_read("+0078", EXESS_SUCCESS, 78, 5);

  // Trailing dot
  check_read("0.1", EXESS_SUCCESS, 0, 1);
  check_read("1.2", EXESS_SUCCESS, 1, 1);
  check_read("2.3", EXESS_SUCCESS, 2, 1);

  // Garbage
  check_read("NaN", EXESS_EXPECTED_DIGIT, 0, 0);
  check_read("INF", EXESS_EXPECTED_DIGIT, 0, 0);
  check_read("-INF", EXESS_EXPECTED_DIGIT, 0, 1);
  check_read("true", EXESS_EXPECTED_DIGIT, 0, 0);
  check_read("+true", EXESS_EXPECTED_DIGIT, 0, 1);
  check_read("-false", EXESS_EXPECTED_DIGIT, 0, 1);

  // Insufficient space for any value
  EXESS_ALIGN char buf[6] = {0};
  assert(exess_read_integer("1", 4, buf).status == EXESS_NO_SPACE);

  // Insufficient space for this value
  assert(exess_read_integer("1234567", sizeof(buf), buf).status ==
         EXESS_OUT_OF_RANGE);
}

/// Check that `str` is a canonical xsd:integer string
static void
check_canonical(const char* const str)
{
  assert(str[0] == '-' || is_digit(str[0]));

  const int first_digit = str[0] == '-' ? 1 : 0;

  for (const char* s = str + first_digit; *s; ++s) {
    assert(*s == '.' || is_digit(*s));
  }
}

static void
check_write_integer(const int64_t     number,
                    const ExessStatus expected_status,
                    const size_t      buf_size,
                    const char* const expected_string)
{
  EXESS_ALIGN char value[VALUE_SIZE] = {0};
  set_integer_value(sizeof(value), value, number);

  char buf[STRING_SIZE] = {42};
  init_out_buf(sizeof(buf), buf);

  assert(buf_size <= sizeof(buf));

  const ExessResult r =
    exess_write_integer(sizeof(value), value, buf_size, buf);
  assert(r.status == expected_status);
  if (!r.status) {
    assert(expected_string);
    assert(r.count == strlen(buf));
    assert(buf[0]);
    assert(!strcmp(buf, expected_string));
    check_canonical(buf);

    assert(exess_write_integer(sizeof(value), value, 0, NULL).count == r.count);

    for (size_t space = 0; space < buf_size; ++space) {
      buf[0] = 1;
      assert(exess_write_integer(sizeof(value), value, space, buf).status ==
             EXESS_NO_SPACE);
      assert(!space || !buf[0]);
    }
  } else {
    assert(!buf[0]);
    assert(exess_write_integer(sizeof(value), value, 0, NULL).count == r.count);
  }
}

static void
test_write_integer(void)
{
  // Numbers
  check_write_integer(-34000, EXESS_SUCCESS, 7, "-34000");
  check_write_integer(-1200, EXESS_SUCCESS, 6, "-1200");
  check_write_integer(0, EXESS_SUCCESS, 2, "0");
  check_write_integer(1, EXESS_SUCCESS, 2, "1");
  check_write_integer(1200, EXESS_SUCCESS, 5, "1200");
  check_write_integer(34000, EXESS_SUCCESS, 6, "34000");
  check_write_integer(34000, EXESS_NO_SPACE, 5, "34000");

  // Bad value size
  EXESS_ALIGN char value[VALUE_SIZE] = {0};
  char             buf[STRING_SIZE]  = {0};
  set_integer_value(sizeof(value), value, 2);
  assert(exess_write_integer(1, value, sizeof(buf), buf).status ==
         EXESS_BAD_VALUE);
}

static void
check_write_decimal(const double      number,
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
    exess_write_integer(sizeof(value), value, buf_size, buf);
  assert(r.status == expected_status);
  if (!r.status) {
    assert(expected_string);
    assert(r.count == strlen(buf));
    assert(buf[0]);
    assert(!strcmp(buf, expected_string));
    check_canonical(buf);

    assert(exess_write_integer(sizeof(value), value, 0, NULL).count == r.count);

    for (size_t space = 0; space < buf_size; ++space) {
      buf[0] = 1;
      assert(exess_write_integer(sizeof(value), value, space, buf).status ==
             EXESS_NO_SPACE);
      assert(!space || !buf[0]);
    }
  } else {
    assert(!buf[0]);
    assert(exess_write_integer(sizeof(value), value, 0, NULL).count == r.count);
  }
}

static void
test_write_decimal(void)
{
  check_write_decimal((double)-INFINITY, EXESS_BAD_VALUE, 1, "");
  check_write_decimal((double)INFINITY, EXESS_BAD_VALUE, 1, "");
  check_write_decimal((double)NAN, EXESS_BAD_VALUE, 1, "");
  check_write_decimal(0.01, EXESS_BAD_VALUE, 1, "");
  check_write_decimal(1.0, EXESS_SUCCESS, 2, "1");
  check_write_decimal(1.5, EXESS_BAD_VALUE, 1, "");
}

static void
check_round_trip(const int64_t number)
{
  EXESS_ALIGN uint8_t value[VALUE_SIZE]        = {0};
  EXESS_ALIGN uint8_t parsed_value[VALUE_SIZE] = {0};
  char                buf[STRING_SIZE]         = {0};

  set_integer_value(sizeof(value), value, number);
  init_out_buf(sizeof(buf), buf);

  const ExessResult w =
    exess_write_integer(sizeof(value), value, sizeof(buf), buf);

  const ExessVariableResult r =
    exess_read_integer(buf, sizeof(parsed_value), parsed_value);

  assert(!w.status);
  assert(!r.status);
  assert(r.read_count == w.count);
  assert(!memcmp(parsed_value, value, r.write_count));
}

static void
test_round_trip(const ExessNumTestOptions opts)
{
  check_round_trip(-1);
  check_round_trip(0);
  check_round_trip(1);

  fprintf(stderr, "Testing xsd:integer randomly with seed %u\n", opts.seed);

  uint64_t state = opts.seed;
  for (uint64_t i = 0; i < opts.n_tests; ++i) {
    state = lcg64(state);

    const uint64_t clamped = (state < INT64_MAX) ? state : INT64_MAX;
    const int64_t  value   = (int64_t)clamped - (INT64_MAX / 2U);

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
    test_read_integer();
    test_write_integer();
    test_write_decimal();
    test_round_trip(opts);
  }

  return (int)opts.error;
}
