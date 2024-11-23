// Copyright 2019-2024 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#ifndef EXESS_TEST_INT_TEST_UTILS_H
#define EXESS_TEST_INT_TEST_UTILS_H

#include <stdint.h>

/// Linear Congruential Generator for making random 32-bit integers
static inline uint32_t
lcg32(const uint32_t i)
{
  static const uint32_t a = 134775813U;
  static const uint32_t c = 1U;

  return (a * i) + c;
}

/// Linear Congruential Generator for making random 64-bit integers
static inline uint64_t
lcg64(const uint64_t i)
{
  static const uint64_t a = 6364136223846793005ULL;
  static const uint64_t c = 1ULL;

  return (a * i) + c;
}

#endif // EXESS_TEST_INT_TEST_UTILS_H
