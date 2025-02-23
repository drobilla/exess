// Copyright 2019-2021 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#include "int_math.h"

#include "exess_config.h"

#include <assert.h>

unsigned
exess_clz32(const uint32_t i)
{
  assert(i != 0);

#if USE_BUILTIN_CLZ
  return (unsigned)__builtin_clz(i);
#else
  unsigned n    = 32U;
  uint32_t bits = i;
  for (unsigned s = 16; s > 0; s >>= 1) {
    const uint32_t left = bits >> s;
    if (left) {
      n -= s;
      bits = left;
    }
  }
  return n - bits;
#endif
}

unsigned
exess_clz64(const uint64_t i)
{
  assert(i != 0);

#if USE_BUILTIN_CLZLL
  return (unsigned)__builtin_clzll(i);
#else
  return (i & 0xFFFFFFFF00000000) ? exess_clz32((uint32_t)(i >> 32U))
                                  : 32U + exess_clz32(i & 0xFFFFFFFF);
#endif
}

uint64_t
exess_ilog2(const uint64_t i)
{
  assert(i != 0);
  return (64U - exess_clz64(i | 1U)) - 1U;
}

uint64_t
exess_ilog10(const uint64_t i)
{
  // See https://graphics.stanford.edu/~seander/bithacks.html#IntegerLog10
  const uint64_t log2 = exess_ilog2(i);
  const uint64_t t    = (log2 + 1U) * 1233U >> 12U;

  return t - (i < POW10[t]) + (i == 0);
}

uint8_t
exess_num_digits(const uint64_t i)
{
  return i == 0U ? 1U : (uint8_t)(exess_ilog10(i) + 1U);
}
