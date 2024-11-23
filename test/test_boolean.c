// Copyright 2011-2021 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#undef NDEBUG

#include <exess/exess.h>

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

static void
check_read(const char* const string,
           const ExessStatus expected_status,
           const bool        expected_value,
           const size_t      expected_count)
{
  bool value = false;

  const ExessResult r = exess_read_boolean(&value, string);
  assert(value == expected_value);
  assert(r.status == expected_status);
  assert(r.count == expected_count);
}

static void
test_read_boolean(void)
{
  // No input
  check_read("", EXESS_EXPECTED_BOOLEAN, false, 0);
  check_read(" \f\n\r\t\v", EXESS_EXPECTED_BOOLEAN, false, 6);

  // Canonical form
  check_read("false", EXESS_SUCCESS, false, 5);
  check_read("true", EXESS_SUCCESS, true, 4);

  // Non-canonical form
  check_read("0", EXESS_SUCCESS, false, 1);
  check_read("1", EXESS_SUCCESS, true, 1);
  check_read(" \f\n\r\t\vfalse ", EXESS_SUCCESS, false, 11);
  check_read(" \f\n\r\t\vtrue ", EXESS_SUCCESS, true, 10);
  check_read(" \f\n\r\t\v0 ", EXESS_SUCCESS, false, 7);
  check_read(" \f\n\r\t\v1 ", EXESS_SUCCESS, true, 7);

  // Trailing garbage
  check_read("falsely", EXESS_EXPECTED_END, false, 5);
  check_read("truely", EXESS_EXPECTED_END, true, 4);
  check_read("0no", EXESS_EXPECTED_END, false, 1);
  check_read("1yes", EXESS_EXPECTED_END, true, 1);

  // Garbage
  check_read("twue", EXESS_EXPECTED_BOOLEAN, false, 0);
  check_read("fawse", EXESS_EXPECTED_BOOLEAN, false, 0);
  check_read("tr", EXESS_EXPECTED_BOOLEAN, false, 0);
  check_read("fa", EXESS_EXPECTED_BOOLEAN, false, 0);
  check_read("yes", EXESS_EXPECTED_BOOLEAN, false, 0);
  check_read("no", EXESS_EXPECTED_BOOLEAN, false, 0);
}

static void
check_write(const bool        value,
            const ExessStatus expected_status,
            const size_t      buf_size,
            const char* const expected_string)
{
  char buf[EXESS_MAX_BOOLEAN_LENGTH + 1] = {1, 2, 3, 4, 5, 0};

  assert(buf_size <= sizeof(buf));

  const ExessResult r = exess_write_boolean(value, buf_size, buf);
  assert(!strcmp(buf, expected_string));
  assert(r.status == expected_status);
  assert(r.count == strlen(buf));
  assert(r.status || exess_write_boolean(value, 0, NULL).count == r.count);
}

static void
test_write_boolean(void)
{
  check_write(true, EXESS_SUCCESS, 5, "true");
  check_write(false, EXESS_SUCCESS, 6, "false");

  check_write(true, EXESS_NO_SPACE, 4, "");
  check_write(false, EXESS_NO_SPACE, 5, "");

  // Check that nothing is written when there isn't enough space
  char              c = 42;
  const ExessResult r = exess_write_boolean(false, 0, &c);
  assert(c == 42);
  assert(r.status == EXESS_NO_SPACE);
  assert(r.count == 0);
}

int
main(void)
{
  test_read_boolean();
  test_write_boolean();
  return 0;
}
