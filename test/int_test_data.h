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
