// Copyright 2011-2021 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#undef NDEBUG

#include "int_test_data.h"
#include "num_test_utils.h"

#include "exess/exess.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static void
check_read(const char* const string,
           const ExessStatus expected_status,
           const int16_t     expected_value,
           const size_t      expected_count)
{
  int16_t value = 0;

  const ExessResult r = exess_read_short(&value, string);
  assert(value == expected_value);
  assert(r.status == expected_status);
  assert(r.count == expected_count);
}

static void
test_read_short(void)
{
  // Limits
  check_read("-32768", EXESS_SUCCESS, INT16_MIN, EXESS_MAX_SHORT_LENGTH);
  check_read("32767", EXESS_SUCCESS, INT16_MAX, 5);

  // Out of range
  check_read("-32769", EXESS_OUT_OF_RANGE, 0, 6);
  check_read("32768", EXESS_OUT_OF_RANGE, 0, 5);

  // Garbage
  check_read("+", EXESS_EXPECTED_DIGIT, 0, 1);
}

static void
check_write(const int16_t     value,
            const ExessStatus expected_status,
            const size_t      buf_size,
            const char* const expected_string)
{
  char buf[EXESS_MAX_SHORT_LENGTH + 1] = {1, 2, 3, 4, 5, 6};

  assert(buf_size <= sizeof(buf));

  const ExessResult r = exess_write_short(value, buf_size, buf);
  assert(!strcmp(buf, expected_string));
  assert(r.status == expected_status);
  assert(r.count == strlen(buf));
  assert(r.status || exess_write_short(value, 0, NULL).count == r.count);
}

static void
test_write_short(void)
{
  check_write(INT16_MIN, EXESS_SUCCESS, 7, "-32768");
  check_write(INT16_MAX, EXESS_SUCCESS, 6, "32767");
}

static void
test_round_trip(const ExessNumTestOptions opts)
{
  int16_t parsed_value                    = 0;
  char    buf[EXESS_MAX_SHORT_LENGTH + 1] = {1, 2, 3, 4, 5, 6};

  if (opts.exhaustive) {
    fprintf(stderr, "Testing xsd:short exhaustively\n");

    for (int64_t i = opts.low; i <= opts.high; ++i) {
      assert(!exess_write_short((int16_t)i, sizeof(buf), buf).status);
      assert(!exess_read_short(&parsed_value, buf).status);
      assert(parsed_value == i);
    }
  } else {
    fprintf(stderr, "Testing xsd:short randomly with seed %u\n", opts.seed);

    uint32_t seed = opts.seed;
    for (uint64_t i = 0; i < opts.n_tests; ++i) {
      seed = lcg32(seed);

      const int16_t value = (int16_t)(seed % INT16_MAX);

      assert(!exess_write_short(value, sizeof(buf), buf).status);
      assert(!exess_read_short(&parsed_value, buf).status);
      assert(parsed_value == value);

      print_num_test_progress(i, opts.n_tests);
    }
  }
}

int
main(int argc, char** argv)
{
  const ExessNumTestOptions opts =
    parse_num_test_options(argc, argv, INT16_MIN, INT16_MAX);

  if (!opts.error) {
    test_read_short();
    test_write_short();
    test_round_trip(opts);
  }

  return (int)opts.error;
}
