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
           const uint8_t     expected_value,
           const size_t      expected_count)
{
  uint8_t value = 0;

  const ExessResult r = exess_read_ubyte(&value, string);
  assert(r.status == expected_status);
  assert(r.count == expected_count);
  assert(value == expected_value);
}

static void
test_read_ubyte(void)
{
  // Limits
  check_read("0", EXESS_SUCCESS, 0, 1);
  check_read("255", EXESS_SUCCESS, UINT8_MAX, EXESS_MAX_UBYTE_LENGTH);

  // Out of range
  check_read("256", EXESS_OUT_OF_RANGE, 0, 3);

  // Garbage
  check_read("-1", EXESS_EXPECTED_DIGIT, 0, 0);
  check_read("+", EXESS_EXPECTED_DIGIT, 0, 0);
}

static void
check_write(const uint8_t     value,
            const ExessStatus expected_status,
            const size_t      buf_size,
            const char* const expected_string)
{
  char buf[EXESS_MAX_UBYTE_LENGTH + 1] = {1, 2, 3, 0};

  assert(buf_size <= sizeof(buf));

  const ExessResult r = exess_write_ubyte(value, buf_size, buf);
  assert(r.status == expected_status);
  assert(r.count == strlen(buf));
  assert(!strcmp(buf, expected_string));
  assert(r.status || exess_write_ubyte(value, 0, NULL).count == r.count);
}

static void
test_write_ubyte(void)
{
  check_write(0, EXESS_SUCCESS, 2, "0");
  check_write(UINT8_MAX, EXESS_SUCCESS, 4, "255");
}

static void
test_round_trip(void)
{
  uint8_t value                           = 0;
  char    buf[EXESS_MAX_UBYTE_LENGTH + 1] = {1, 2, 3, 4};

  for (uint16_t i = 0; i <= UINT8_MAX; ++i) {
    assert(!exess_write_ubyte((uint8_t)i, sizeof(buf), buf).status);
    assert(!exess_read_ubyte(&value, buf).status);
    assert(value == i);
  }
}

int
main(void)
{
  test_read_ubyte();
  test_write_ubyte();
  test_round_trip();

  return 0;
}
