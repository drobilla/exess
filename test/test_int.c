// Copyright 2011-2021 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#undef NDEBUG

#include "int_test_data.h"
#include "num_test_utils.h"

#include "exess/exess.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static void
check_read(const char* const string,
           const ExessStatus expected_status,
           const int32_t     expected_value,
           const size_t      expected_count)
{
  int32_t           value = 0;
  const ExessResult r     = exess_read_int(&value, string);

  assert(r.status == expected_status);
  assert(r.count == expected_count);
  assert(value == expected_value);
}

static void
test_read_int(void)
{
  // Limits
  check_read("-2147483648", EXESS_SUCCESS, INT32_MIN, EXESS_MAX_INT_LENGTH);
  check_read("2147483647", EXESS_SUCCESS, INT32_MAX, 10);

  // Out of range
  check_read("-2147483649", EXESS_OUT_OF_RANGE, 0, 11);
  check_read("2147483648", EXESS_OUT_OF_RANGE, 0, 10);
  check_read("10000000000", EXESS_OUT_OF_RANGE, 0, 11);

  // Garbage
  check_read("+", EXESS_EXPECTED_DIGIT, 0, 1);
}

static void
check_write(const int32_t     value,
            const ExessStatus expected_status,
            const size_t      buf_size,
            const char* const expected_string)
{
  char buf[EXESS_MAX_INT_LENGTH + 1] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};

  assert(buf_size <= sizeof(buf));

  const ExessResult r = exess_write_int(value, buf_size, buf);
  assert(!strcmp(buf, expected_string));
  assert(r.status == expected_status);
  assert(r.count == strlen(buf));
  assert(r.status || exess_write_int(value, 0, NULL).count == r.count);
}

static void
test_write_int(void)
{
  check_write(INT32_MIN, EXESS_SUCCESS, 12, "-2147483648");
  check_write(INT32_MAX, EXESS_SUCCESS, 11, "2147483647");
}

static void
test_round_trip(const ExessNumTestOptions opts)
{
  int32_t parsed_value               = 0;
  char buf[EXESS_MAX_INT_LENGTH + 1] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};

  if (opts.exhaustive) {
    fprintf(stderr, "Testing xsd:int exhaustively\n");

    for (int64_t i = opts.low; i <= opts.high; ++i) {
      assert(!exess_write_int((int32_t)i, sizeof(buf), buf).status);
      assert(!exess_read_int(&parsed_value, buf).status);
      assert(parsed_value == i);

      print_num_test_progress((uint64_t)(i - (int64_t)INT32_MIN), UINT32_MAX);
    }
  } else {
    fprintf(stderr, "Testing xsd:int randomly with seed %u\n", opts.seed);
    uint32_t rep = opts.seed;
    for (uint64_t i = 0; i < opts.n_tests; ++i) {
      rep = lcg32(rep);

      const int32_t value = (int32_t)rep;

      assert(!exess_write_int(value, sizeof(buf), buf).status);
      assert(!exess_read_int(&parsed_value, buf).status);
      assert(parsed_value == value);

      print_num_test_progress(i, opts.n_tests);
    }
  }
}

int
main(int argc, char** argv)
{
  const ExessNumTestOptions opts =
    parse_num_test_options(argc, argv, 16384U, INT32_MIN, INT32_MAX);

  if (!opts.error) {
    test_read_int();
    test_write_int();
    test_round_trip(opts);
  }

  return (int)opts.error;
}
