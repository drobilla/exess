// Copyright 2019-2021 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#ifndef EXESS_INTMATH_H
#define EXESS_INTMATH_H

#include "attributes.h"

#include <stdint.h>

static const int uint64_digits10 = 19;

static const uint64_t POW10[] = {1ull,
                                 10ull,
                                 100ull,
                                 1000ull,
                                 10000ull,
                                 100000ull,
                                 1000000ull,
                                 10000000ull,
                                 100000000ull,
                                 1000000000ull,
                                 10000000000ull,
                                 100000000000ull,
                                 1000000000000ull,
                                 10000000000000ull,
                                 100000000000000ull,
                                 1000000000000000ull,
                                 10000000000000000ull,
                                 100000000000000000ull,
                                 1000000000000000000ull,
                                 10000000000000000000ull};

/// Return the number of leading zeros in `i`
EXESS_I_CONST_FUNC
unsigned
exess_clz32(uint32_t i);

/// Return the number of leading zeros in `i`
EXESS_I_CONST_FUNC
unsigned
exess_clz64(uint64_t i);

/// Return the log base 2 of `i`
EXESS_I_CONST_FUNC
uint64_t
exess_ilog2(uint64_t i);

/// Return the log base 10 of `i`
EXESS_I_CONST_FUNC
uint64_t
exess_ilog10(uint64_t i);

/// Return the number of decimal digits required to represent `i`
EXESS_I_CONST_FUNC
uint8_t
exess_num_digits(uint64_t i);

#endif // EXESS_INTMATH_H
