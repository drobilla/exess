// Copyright 2011-2021 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#undef NDEBUG

#include <exess/exess.h>

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

static void
check_read(const char* const string,
           const ExessStatus expected_status,
           const uint64_t    expected_value,
           const size_t      expected_count)
{
  uint64_t          value = 0;
  const ExessResult r     = exess_read_ulong(&value, string);

  assert(r.status == expected_status);
  assert(r.count == expected_count);
  assert(value == expected_value);
}

static void
test_read_ulong(void)
{
  // No input
  check_read("", EXESS_EXPECTED_DIGIT, 0, 0);
  check_read(" \f\n\r\t\v", EXESS_EXPECTED_DIGIT, 0, 6);

  // Canonical form
  check_read("0", EXESS_SUCCESS, 0, 1);
  check_read("1234", EXESS_SUCCESS, 1234, 4);

  // Non-canonical form
  check_read(" \f\n\r\t\v1234 ", EXESS_SUCCESS, 1234, 10);
  check_read(" \f\n\r\t\v01234 ", EXESS_SUCCESS, 1234, 11);
  check_read("01234", EXESS_SUCCESS, 1234, 5);
  check_read("00", EXESS_SUCCESS, 0, 2);

  // Limits
  check_read("0", EXESS_SUCCESS, 0, 1);
  check_read(
    "18446744073709551615", EXESS_SUCCESS, UINT64_MAX, EXESS_MAX_ULONG_LENGTH);

  // Out of range
  check_read("18446744073709551616", EXESS_OUT_OF_RANGE, 0, 19);

  // Trailing garbage
  check_read("1234extra", EXESS_EXPECTED_END, 1234, 4);

  // Garbage
  check_read(" \f\n\r\t\v+1234 ", EXESS_EXPECTED_DIGIT, 0, 6);
  check_read("+1234", EXESS_EXPECTED_DIGIT, 0, 0);
  check_read("+0", EXESS_EXPECTED_DIGIT, 0, 0);
  check_read("+", EXESS_EXPECTED_DIGIT, 0, 0);
  check_read("-", EXESS_EXPECTED_DIGIT, 0, 0);
  check_read("true", EXESS_EXPECTED_DIGIT, 0, 0);
  check_read("false", EXESS_EXPECTED_DIGIT, 0, 0);
  check_read("zero", EXESS_EXPECTED_DIGIT, 0, 0);
  check_read("NaN", EXESS_EXPECTED_DIGIT, 0, 0);
  check_read("INF", EXESS_EXPECTED_DIGIT, 0, 0);
  check_read("-INF", EXESS_EXPECTED_DIGIT, 0, 0);
}

static void
check_write(const uint64_t    value,
            const ExessStatus expected_status,
            const size_t      buf_size,
            const char* const expected_string)
{
  char buf[EXESS_MAX_ULONG_LENGTH + 1] = {
    1,  2,  3,  4,  5,  6,  7,  8,  9,  10, //
    11, 12, 13, 14, 15, 16, 17, 18, 19, 20, //
    0};

  assert(buf_size <= sizeof(buf));

  const ExessResult r = exess_write_ulong(value, buf_size, buf);

  assert(r.status == expected_status);
  assert(r.count == strlen(buf));
  assert(!strcmp(buf, expected_string));
  assert(r.status || exess_write_ulong(value, 0, NULL).count == r.count);
}

static void
test_write_ulong(void)
{
  check_write(0U, EXESS_SUCCESS, 2, "0");
  check_write(1U, EXESS_SUCCESS, 2, "1");
  check_write(UINT64_MAX, EXESS_SUCCESS, 21, "18446744073709551615");

  check_write(1234U, EXESS_NO_SPACE, 4, "");

  // Check that nothing is written when there isn't enough space
  char              c = 42;
  const ExessResult r = exess_write_ulong(1234U, 0, &c);
  assert(c == 42);
  assert(r.status == EXESS_NO_SPACE);
  assert(r.count == 0);
}

int
main(void)
{
  test_read_ulong();
  test_write_ulong();
  return 0;
}
