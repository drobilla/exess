// Copyright 2019-2021 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#undef NDEBUG

#include "../src/int_math.h"

#include <assert.h>
#include <stdint.h>

static void
test_clz32(void)
{
  for (unsigned i = 0; i < 32; ++i) {
    assert(exess_clz32(1U << i) == 32U - i - 1U);
  }
}

static void
test_clz64(void)
{
  for (unsigned i = 0; i < 64; ++i) {
    assert(exess_clz64(1ULL << i) == 64U - i - 1U);
  }
}

static void
test_ilog2(void)
{
  for (unsigned i = 0; i < 64; ++i) {
    assert(exess_ilog2(1ULL << i) == i);
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
  assert(2 == exess_num_digits(99ULL));
  assert(3 == exess_num_digits(999ULL));
  assert(4 == exess_num_digits(9999ULL));
  assert(5 == exess_num_digits(99999ULL));
  assert(6 == exess_num_digits(999999ULL));
  assert(7 == exess_num_digits(9999999ULL));
  assert(8 == exess_num_digits(99999999ULL));
  assert(9 == exess_num_digits(999999999ULL));
  assert(10 == exess_num_digits(9999999999ULL));
  assert(11 == exess_num_digits(99999999999ULL));
  assert(12 == exess_num_digits(999999999999ULL));
  assert(13 == exess_num_digits(9999999999999ULL));
  assert(14 == exess_num_digits(99999999999999ULL));
  assert(15 == exess_num_digits(999999999999999ULL));
  assert(16 == exess_num_digits(9999999999999999ULL));
  assert(17 == exess_num_digits(99999999999999999ULL));
  assert(18 == exess_num_digits(999999999999999999ULL));
  assert(19 == exess_num_digits(9999999999999999999ULL));
  assert(20 == exess_num_digits(18446744073709551615ULL));
}

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
