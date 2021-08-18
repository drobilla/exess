// Copyright 2019-2021 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#ifndef INT_TEST_DATA_H
#define INT_TEST_DATA_H

#include <stdint.h>

/// Linear Congruential Generator for making random 32-bit integers
static inline uint32_t
lcg32(const uint32_t i)
{
  static const uint32_t a = 134775813u;
  static const uint32_t c = 1u;

  return (a * i) + c;
}

/// Linear Congruential Generator for making random 64-bit integers
static inline uint64_t
lcg64(const uint64_t i)
{
  static const uint64_t a = 6364136223846793005ull;
  static const uint64_t c = 1ull;

  return (a * i) + c;
}

#endif // INT_TEST_DATA_H
