// Copyright 2019-2023 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#ifndef EXESS_SRC_INTMATH_H
#define EXESS_SRC_INTMATH_H

#include "attributes.h"

#include <stdint.h>

/// Powers of 10 indexed by exponent, so POW10[e] == 10^e
static const uint64_t POW10[] = {1ULL,
                                 10ULL,
                                 100ULL,
                                 1000ULL,
                                 10000ULL,
                                 100000ULL,
                                 1000000ULL,
                                 10000000ULL,
                                 100000000ULL,
                                 1000000000ULL,
                                 10000000000ULL,
                                 100000000000ULL,
                                 1000000000000ULL,
                                 10000000000000ULL,
                                 100000000000000ULL,
                                 1000000000000000ULL,
                                 10000000000000000ULL,
                                 100000000000000000ULL,
                                 1000000000000000000ULL,
                                 10000000000000000000ULL};

/// Return the number of leading zeros in `i`
EXESS_I_CONST_FUNC unsigned
exess_clz32(uint32_t i);

/// Return the number of leading zeros in `i`
EXESS_I_CONST_FUNC unsigned
exess_clz64(uint64_t i);

/// Return the log base 2 of `i`
EXESS_I_CONST_FUNC uint64_t
exess_ilog2(uint64_t i);

/// Return the log base 10 of `i`
EXESS_I_CONST_FUNC uint64_t
exess_ilog10(uint64_t i);

/// Return the number of decimal digits required to represent `i`
EXESS_I_CONST_FUNC uint8_t
exess_num_digits(uint64_t i);

#endif // EXESS_SRC_INTMATH_H
