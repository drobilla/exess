/*
  Copyright 2011-2021 David Robillard <d@drobilla.net>

  Permission to use, copy, modify, and/or distribute this software for any
  purpose with or without fee is hereby granted, provided that the above
  copyright notice and this permission notice appear in all copies.

  THIS SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
  WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
  MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
  ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
  WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
  ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
  OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/

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
           const uint32_t    expected_value,
           const size_t      expected_count)
{
  uint32_t          value = 0;
  const ExessResult r     = exess_read_uint(&value, string);

  assert(r.status == expected_status);
  assert(r.count == expected_count);
  assert(value == expected_value);
}

static void
test_read_uint(void)
{
  // Limits
  check_read("0", EXESS_SUCCESS, 0, 1);
  check_read("4294967295", EXESS_SUCCESS, UINT32_MAX, EXESS_MAX_UINT_LENGTH);

  // Out of range
  check_read("4294967296", EXESS_OUT_OF_RANGE, 0, 10);

  // Garbage
  check_read("-1", EXESS_EXPECTED_DIGIT, 0, 0);
  check_read("+", EXESS_EXPECTED_DIGIT, 0, 0);
}

static void
check_write(const uint32_t    value,
            const ExessStatus expected_status,
            const size_t      buf_size,
            const char* const expected_string)
{
  char buf[EXESS_MAX_UINT_LENGTH + 1] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};

  assert(buf_size <= sizeof(buf));

  const ExessResult r = exess_write_uint(value, buf_size, buf);
  assert(r.status == expected_status);
  assert(r.count == strlen(buf));
  assert(!strcmp(buf, expected_string));
  assert(r.status || exess_write_uint(value, 0, NULL).count == r.count);
}

static void
test_write_uint(void)
{
  check_write(0u, EXESS_SUCCESS, 2, "0");
  check_write(UINT32_MAX, EXESS_SUCCESS, 11, "4294967295");
}

static void
test_round_trip(const ExessNumTestOptions opts)
{
  uint32_t parsed_value                   = 0;
  char     buf[EXESS_MAX_UINT_LENGTH + 1] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};

  if (opts.exhaustive) {
    fprintf(stderr, "Testing xsd:unsignedInt exhaustively\n");

    for (uint64_t i = 0u; i <= UINT32_MAX; ++i) {
      assert(!exess_write_uint((uint32_t)i, sizeof(buf), buf).status);
      assert(!exess_read_uint(&parsed_value, buf).status);
      assert(parsed_value == i);

      print_num_test_progress(i, UINT32_MAX);
    }
  } else {
    fprintf(
      stderr, "Testing xsd:unsignedInt randomly with seed %u\n", opts.seed);

    uint32_t value = opts.seed;
    for (uint64_t i = 0; i < opts.n_tests; ++i) {
      value = lcg32(value);

      assert(!exess_write_uint(value, sizeof(buf), buf).status);
      assert(!exess_read_uint(&parsed_value, buf).status);
      assert(parsed_value == value);

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

  test_read_uint();
  test_write_uint();
  test_round_trip(opts);

  return 0;
}
