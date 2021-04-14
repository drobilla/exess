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

#include "exess/exess.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

static void
check_read(const char* const string,
           const ExessStatus expected_status,
           const int8_t      expected_value,
           const size_t      expected_count)
{
  int8_t            value = 0;
  const ExessResult r     = exess_read_byte(&value, string);

  assert(r.status == expected_status);
  assert(r.count == expected_count);
  assert(value == expected_value);
}

static void
test_read_byte(void)
{
  // Limits
  check_read("-128", EXESS_SUCCESS, INT8_MIN, EXESS_MAX_BYTE_LENGTH);
  check_read("127", EXESS_SUCCESS, INT8_MAX, 3);

  // Out of range
  check_read("-129", EXESS_OUT_OF_RANGE, 0, 4);
  check_read("128", EXESS_OUT_OF_RANGE, 0, 3);

  // Garbage
  check_read("+", EXESS_EXPECTED_DIGIT, 0, 1);
}

static void
check_write(const int8_t      value,
            const ExessStatus expected_status,
            const size_t      buf_size,
            const char* const expected_string)
{
  char buf[EXESS_MAX_BYTE_LENGTH + 1] = {1, 2, 3, 4, 5};
  assert(buf_size <= sizeof(buf));

  const ExessResult r = exess_write_byte(value, buf_size, buf);
  assert(r.status == expected_status);
  assert(!strcmp(buf, expected_string));
  assert(r.count == strlen(buf));
  assert(r.status || exess_write_byte(value, 0, NULL).count == r.count);
}

static void
test_write_byte(void)
{
  check_write(INT8_MIN, EXESS_SUCCESS, 5, "-128");
  check_write(INT8_MAX, EXESS_SUCCESS, 4, "127");
}

static void
test_round_trip(void)
{
  int8_t value                          = 0;
  char   buf[EXESS_MAX_BYTE_LENGTH + 1] = {1, 2, 3, 4, 5};

  for (int16_t i = INT8_MIN; i <= INT8_MAX; ++i) {
    assert(!exess_write_byte((int8_t)i, sizeof(buf), buf).status);
    assert(!exess_read_byte(&value, buf).status);
    assert(value == i);
  }
}

int
main(void)
{
  test_read_byte();
  test_write_byte();
  test_round_trip();
  return 0;
}
