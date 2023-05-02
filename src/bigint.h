// Copyright 2019-2021 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#ifndef EXESS_SRC_BIGINT_H
#define EXESS_SRC_BIGINT_H

#include "attributes.h"

#include <stdint.h>

typedef uint32_t Bigit;

/* We need enough precision for any double, the "largest" of which (using
   absolute exponents) is the smallest subnormal ~= 5e-324.  This is 1076 bits
   long, but we need a bit more space for arithmetic.  This is absurd, but such
   is decimal.  These are only used on the stack so it doesn't hurt too much.
*/

#define BIGINT_MAX_SIGNIFICANT_BITS 1280U
#define BIGINT_BIGIT_BITS 32U
#define BIGINT_MAX_BIGITS (BIGINT_MAX_SIGNIFICANT_BITS / BIGINT_BIGIT_BITS)

typedef struct {
  Bigit    bigits[BIGINT_MAX_BIGITS];
  unsigned n_bigits;
} ExessBigint;

void
exess_bigint_zero(ExessBigint* num);

void
exess_bigint_clamp(ExessBigint* num);

void
exess_bigint_shift_left(ExessBigint* num, unsigned amount);

void
exess_bigint_set(ExessBigint* num, const ExessBigint* value);

void
exess_bigint_set_u32(ExessBigint* num, uint32_t value);

void
exess_bigint_set_u64(ExessBigint* num, uint64_t value);

void
exess_bigint_set_pow10(ExessBigint* num, unsigned exponent);

void
exess_bigint_set_decimal_string(ExessBigint* num, const char* str);

void
exess_bigint_multiply_u32(ExessBigint* num, uint32_t factor);

void
exess_bigint_multiply_u64(ExessBigint* num, uint64_t factor);

void
exess_bigint_multiply_pow10(ExessBigint* num, unsigned exponent);

EXESS_I_PURE_FUNC
int
exess_bigint_compare(const ExessBigint* lhs, const ExessBigint* rhs);

void
exess_bigint_add_u32(ExessBigint* lhs, uint32_t rhs);

void
exess_bigint_add(ExessBigint* lhs, const ExessBigint* rhs);

void
exess_bigint_subtract(ExessBigint* lhs, const ExessBigint* rhs);

EXESS_I_PURE_FUNC
Bigit
exess_bigint_left_shifted_bigit(const ExessBigint* num,
                                unsigned           amount,
                                unsigned           index);

/// Faster implementation of exess_bigint_subtract(lhs, rhs << amount)
void
exess_bigint_subtract_left_shifted(ExessBigint*       lhs,
                                   const ExessBigint* rhs,
                                   unsigned           amount);

/// Faster implementation of exess_bigint_compare(l + p, c)
EXESS_I_PURE_FUNC
int
exess_bigint_plus_compare(const ExessBigint* l,
                          const ExessBigint* p,
                          const ExessBigint* c);

/// Divide and set `lhs` to modulo
uint32_t
exess_bigint_divmod(ExessBigint* lhs, const ExessBigint* rhs);

#endif // EXESS_SRC_BIGINT_H
