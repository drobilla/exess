// Copyright 2011-2021 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#undef NDEBUG

#include "exess/exess.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

static void
check_read(const char* const string,
           const ExessStatus expected_status,
           const uint16_t    expected_value,
           const size_t      expected_count)
{
  uint16_t value = 0;

  const ExessResult r = exess_read_ushort(&value, string);
  assert(value == expected_value);
  assert(r.status == expected_status);
  assert(r.count == expected_count);
}

static void
test_read_ushort(void)
{
  // Limits
  check_read("0", EXESS_SUCCESS, 0, 1);
  check_read("65535", EXESS_SUCCESS, UINT16_MAX, EXESS_MAX_USHORT_LENGTH);

  // Out of range
  check_read("65536", EXESS_OUT_OF_RANGE, 0, 5);

  // Garbage
  check_read("-1", EXESS_EXPECTED_DIGIT, 0, 0);
  check_read("+", EXESS_EXPECTED_DIGIT, 0, 0);
}

static void
check_write(const uint16_t    value,
            const ExessStatus expected_status,
            const size_t      buf_size,
            const char* const expected_string)
{
  char buf[EXESS_MAX_USHORT_LENGTH + 1] = {1, 2, 3, 4, 5, 6};

  assert(buf_size <= sizeof(buf));

  const ExessResult r = exess_write_ushort(value, buf_size, buf);
  assert(!strcmp(buf, expected_string));
  assert(r.status == expected_status);
  assert(r.count == strlen(buf));
  assert(r.status || exess_write_ushort(value, 0, NULL).count == r.count);
}

static void
test_write_ushort(void)
{
  check_write(0u, EXESS_SUCCESS, 2, "0");
  check_write(UINT16_MAX, EXESS_SUCCESS, 6, "65535");
}

static void
test_round_trip(void)
{
  uint16_t value                            = 0;
  char     buf[EXESS_MAX_USHORT_LENGTH + 1] = {1, 2, 3, 4, 5, 6};

  for (uint32_t i = 0; i <= UINT16_MAX; ++i) {
    assert(!exess_write_ushort((uint16_t)i, sizeof(buf), buf).status);
    assert(!exess_read_ushort(&value, buf).status);
    assert(value == i);
  }
}

int
main(void)
{
  test_read_ushort();
  test_write_ushort();
  test_round_trip();

  return 0;
}
