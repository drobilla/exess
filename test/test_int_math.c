/*
  Copyright 2019-2021 David Robillard <d@drobilla.net>

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

#include "attributes.h"
#include "int_math.h"

#include <assert.h>
#include <stdint.h>

static void
test_clz32(void)
{
  for (unsigned i = 0; i < 32; ++i) {
    assert(exess_clz32(1u << i) == 32u - i - 1u);
  }
}

static void
test_clz64(void)
{
  for (unsigned i = 0; i < 64; ++i) {
    assert(exess_clz64(1ull << i) == 64u - i - 1u);
  }
}

static void
test_ilog2(void)
{
  for (unsigned i = 0; i < 64; ++i) {
    assert(exess_ilog2(1ull << i) == i);
  }
}

static void
test_ilog10(void)
{
  uint64_t power = 1;
  for (unsigned i = 0; i < 20; ++i, power *= 10) {
    assert(exess_ilog10(power) == i);
  }
}

static void
test_num_digits(void)
{
  assert(1 == exess_num_digits(0));
  assert(1 == exess_num_digits(1));
  assert(1 == exess_num_digits(9));
  assert(2 == exess_num_digits(10));
  assert(2 == exess_num_digits(99ull));
  assert(3 == exess_num_digits(999ull));
  assert(4 == exess_num_digits(9999ull));
  assert(5 == exess_num_digits(99999ull));
  assert(6 == exess_num_digits(999999ull));
  assert(7 == exess_num_digits(9999999ull));
  assert(8 == exess_num_digits(99999999ull));
  assert(9 == exess_num_digits(999999999ull));
  assert(10 == exess_num_digits(9999999999ull));
  assert(11 == exess_num_digits(99999999999ull));
  assert(12 == exess_num_digits(999999999999ull));
  assert(13 == exess_num_digits(9999999999999ull));
  assert(14 == exess_num_digits(99999999999999ull));
  assert(15 == exess_num_digits(999999999999999ull));
  assert(16 == exess_num_digits(9999999999999999ull));
  assert(17 == exess_num_digits(99999999999999999ull));
  assert(18 == exess_num_digits(999999999999999999ull));
  assert(19 == exess_num_digits(9999999999999999999ull));
  assert(20 == exess_num_digits(18446744073709551615ull));
}

EXESS_I_PURE_FUNC
int
main(void)
{
  test_clz32();
  test_clz64();
  test_ilog2();
  test_ilog10();
  test_num_digits();

  return 0;
}
