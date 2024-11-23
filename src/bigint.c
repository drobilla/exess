// Copyright 2019-2021 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#include "bigint.h"
#include "macros.h"

#include "int_math.h"

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>

typedef uint64_t Hugit;

static const uint32_t bigit_mask = ~(uint32_t)0;
static const uint64_t carry_mask = (uint64_t) ~(uint32_t)0 << 32U;

typedef struct {
  unsigned bigits;
  unsigned bits;
} Offset;

static inline Offset
make_offset(const unsigned i)
{
  const unsigned bigits = i / BIGINT_BIGIT_BITS;
  const unsigned bits   = i % BIGINT_BIGIT_BITS;

  const Offset offset = {bigits, bits};
  return offset;
}

#ifndef NDEBUG
static inline bool
exess_bigint_is_clamped(const ExessBigint* num)
{
  return num->n_bigits == 0 || num->bigits[num->n_bigits - 1];
}
#endif

void
exess_bigint_shift_left(ExessBigint* num, const unsigned amount)
{
  assert(exess_bigint_is_clamped(num));
  if (amount == 0 || num->n_bigits == 0) {
    return;
  }

  const Offset offset = make_offset(amount);

  assert(num->n_bigits + offset.bigits < BIGINT_MAX_BIGITS);
  num->n_bigits += offset.bigits + (bool)offset.bits;

  if (offset.bits == 0) { // Simple bigit-aligned shift
    for (unsigned i = num->n_bigits - 1; i >= offset.bigits; --i) {
      num->bigits[i] = num->bigits[i - offset.bigits];
    }
  } else { // Bigit + sub-bigit bit offset shift
    const unsigned right_shift = BIGINT_BIGIT_BITS - offset.bits;
    for (unsigned i = num->n_bigits - offset.bigits - 1; i > 0; --i) {
      num->bigits[i + offset.bigits] =
        (num->bigits[i] << offset.bits) | (num->bigits[i - 1] >> right_shift);
    }

    num->bigits[offset.bigits] = num->bigits[0] << offset.bits;
  }

  // Zero LSBs
  for (unsigned i = 0; i < offset.bigits; ++i) {
    num->bigits[i] = 0;
  }

  exess_bigint_clamp(num);
  assert(exess_bigint_is_clamped(num));
}

void
exess_bigint_zero(ExessBigint* num)
{
  static const ExessBigint zero = {{0}, 0};

  *num = zero;
}

void
exess_bigint_set(ExessBigint* num, const ExessBigint* value)
{
  *num = *value;
}

void
exess_bigint_set_u32(ExessBigint* num, const uint32_t value)
{
  exess_bigint_zero(num);

  num->bigits[0] = value;
  num->n_bigits  = (bool)value;
}

void
exess_bigint_clamp(ExessBigint* num)
{
  while (num->n_bigits > 0 && num->bigits[num->n_bigits - 1] == 0) {
    --num->n_bigits;
  }
}

void
exess_bigint_set_u64(ExessBigint* num, const uint64_t value)
{
  exess_bigint_zero(num);

  num->bigits[0] = (Bigit)(value & bigit_mask);
  num->bigits[1] = (Bigit)(value >> BIGINT_BIGIT_BITS);
  num->n_bigits  = num->bigits[1] ? 2U : num->bigits[0] ? 1U : 0U;
}

void
exess_bigint_set_pow10(ExessBigint* num, const unsigned exponent)
{
  exess_bigint_set_u32(num, 1);
  exess_bigint_multiply_pow10(num, exponent);
}

static uint32_t
read_u32(const char* const str, uint32_t* result, uint32_t* n_digits)
{
  static const size_t uint32_digits10 = 9;

  *result = *n_digits = 0;

  uint32_t i = 0;
  for (; str[i] >= '0' && str[i] <= '9' && *n_digits < uint32_digits10; ++i) {
    *result = *result * 10U + (unsigned)(str[i] - '0');
    *n_digits += 1;
  }

  return i;
}

void
exess_bigint_set_decimal_string(ExessBigint* num, const char* const str)
{
  exess_bigint_zero(num);

  uint32_t pos      = 0;
  uint32_t n_digits = 0;
  uint32_t n_read   = 0;
  uint32_t word     = 0;
  while ((n_read = read_u32(str + pos, &word, &n_digits))) {
    exess_bigint_multiply_u32(num, (uint32_t)POW10[n_digits]);
    exess_bigint_add_u32(num, word);
    pos += n_read;
  }

  exess_bigint_clamp(num);
}

void
exess_bigint_multiply_u32(ExessBigint* num, const uint32_t factor)
{
  switch (factor) {
  case 0:
    exess_bigint_zero(num);
    return;
  case 1:
    return;
  default:
    break;
  }

  Hugit carry = 0;
  for (unsigned i = 0; i < num->n_bigits; ++i) {
    const Hugit p     = (Hugit)factor * num->bigits[i];
    const Hugit hugit = p + (carry & bigit_mask);

    num->bigits[i] = (Bigit)(hugit & bigit_mask);

    carry = (hugit >> 32U) + (carry >> 32U);
  }

  for (; carry; carry >>= 32U) {
    assert(num->n_bigits + 1 <= BIGINT_MAX_BIGITS);
    num->bigits[num->n_bigits++] = (Bigit)carry;
  }
}

void
exess_bigint_multiply_u64(ExessBigint* num, const uint64_t factor)
{
  switch (factor) {
  case 0:
    exess_bigint_zero(num);
    return;
  case 1:
    return;
  default:
    break;
  }

  const Hugit f_lo = factor & bigit_mask;
  const Hugit f_hi = factor >> 32U;

  Hugit carry = 0;
  for (unsigned i = 0; i < num->n_bigits; ++i) {
    const Hugit p_lo  = f_lo * num->bigits[i];
    const Hugit p_hi  = f_hi * num->bigits[i];
    const Hugit hugit = p_lo + (carry & bigit_mask);

    num->bigits[i] = (Bigit)(hugit & bigit_mask);
    carry          = p_hi + (hugit >> 32U) + (carry >> 32U);
  }

  for (; carry; carry >>= 32U) {
    assert(num->n_bigits + 1 <= BIGINT_MAX_BIGITS);
    num->bigits[num->n_bigits++] = (Bigit)(carry & bigit_mask);
  }
}

void
exess_bigint_multiply_pow10(ExessBigint* num, const unsigned exponent)
{
  /* To reduce multiplication, we exploit 10^e = (2*5)^e = 2^e * 5^e to
     factor out an exponentiation by 5 instead of 10.  So, we first multiply
     by 5^e (hard), then by 2^e (just a single left shift). */

  // 5^27, the largest power of 5 that fits in 64 bits
  static const uint64_t pow5_27 = 7450580596923828125ULL;

  // Powers of 5 up to 5^13, the largest that fits in 32 bits
  static const uint32_t pow5[] = {
    1,
    5,
    5 * 5,
    5 * 5 * 5,
    5 * 5 * 5 * 5,
    5 * 5 * 5 * 5 * 5,
    5 * 5 * 5 * 5 * 5 * 5,
    5 * 5 * 5 * 5 * 5 * 5 * 5,
    5 * 5 * 5 * 5 * 5 * 5 * 5 * 5,
    5 * 5 * 5 * 5 * 5 * 5 * 5 * 5 * 5,
    5 * 5 * 5 * 5 * 5 * 5 * 5 * 5 * 5 * 5,
    5 * 5 * 5 * 5 * 5 * 5 * 5 * 5 * 5 * 5 * 5,
    5 * 5 * 5 * 5 * 5 * 5 * 5 * 5 * 5 * 5 * 5 * 5,
    5 * 5 * 5 * 5 * 5 * 5 * 5 * 5 * 5 * 5 * 5 * 5 * 5,
  };

  if (exponent == 0 || num->n_bigits == 0) {
    return;
  }

  // Multiply by 5^27 until e < 27 so we can switch to 32 bits
  unsigned e = exponent;
  while (e >= 27) {
    exess_bigint_multiply_u64(num, pow5_27);
    e -= 27;
  }

  // Multiply by 5^13 until e < 13 so we have only one multiplication left
  while (e >= 13) {
    exess_bigint_multiply_u32(num, pow5[13]);
    e -= 13;
  }

  // Multiply by the final 5^e (which may be zero, making this a noop)
  exess_bigint_multiply_u32(num, pow5[e]);

  // Finally multiply by 2^e
  exess_bigint_shift_left(num, exponent);
}

int
exess_bigint_compare(const ExessBigint* lhs, const ExessBigint* rhs)
{
  if (lhs->n_bigits < rhs->n_bigits) {
    return -1;
  }

  if (lhs->n_bigits > rhs->n_bigits) {
    return 1;
  }

  for (int i = (int)lhs->n_bigits - 1; i >= 0; --i) {
    const Bigit bigit_l = lhs->bigits[i];
    const Bigit bigit_r = rhs->bigits[i];
    if (bigit_l < bigit_r) {
      return -1;
    }

    if (bigit_l > bigit_r) {
      return 1;
    }
  }

  return 0;
}

int
exess_bigint_plus_compare(const ExessBigint* l,
                          const ExessBigint* p,
                          const ExessBigint* c)
{
  assert(exess_bigint_is_clamped(l));
  assert(exess_bigint_is_clamped(p));
  assert(exess_bigint_is_clamped(c));

  if (l->n_bigits < p->n_bigits) {
    return exess_bigint_plus_compare(p, l, c);
  }

  if (l->n_bigits + 1 < c->n_bigits) {
    return -1;
  }

  if (l->n_bigits > c->n_bigits) {
    return 1;
  }

  if (p->n_bigits < l->n_bigits && l->n_bigits < c->n_bigits) {
    return -1;
  }

  Hugit borrow = 0;
  for (int i = (int)c->n_bigits - 1; i >= 0; --i) {
    const Bigit ai  = l->bigits[i];
    const Bigit bi  = p->bigits[i];
    const Bigit ci  = c->bigits[i];
    const Hugit sum = (Hugit)ai + bi;

    if (sum > ci + borrow) {
      return 1;
    }

    borrow += ci - sum;
    if (borrow > 1) {
      return -1;
    }

    borrow <<= 32U;
  }

  return borrow ? -1 : 0;
}

static unsigned
exess_bigint_add_carry(ExessBigint* lhs, unsigned i, bool carry)
{
  for (; carry; ++i) {
    const Hugit sum = (Hugit)carry + lhs->bigits[i];
    lhs->bigits[i]  = (Bigit)(sum & bigit_mask);
    carry           = (Bigit)((sum & carry_mask) >> 32U);
  }

  return i;
}

void
exess_bigint_add_u32(ExessBigint* lhs, const uint32_t rhs)
{
  if (lhs->n_bigits == 0) {
    exess_bigint_set_u32(lhs, rhs);
    return;
  }

  Hugit sum   = (Hugit)lhs->bigits[0] + rhs;
  Bigit carry = (Bigit)(sum >> 32U);

  lhs->bigits[0] = (Bigit)(sum & bigit_mask);

  const unsigned i = exess_bigint_add_carry(lhs, 1U, carry);
  lhs->n_bigits    = MAX(i, lhs->n_bigits);
  assert(exess_bigint_is_clamped(lhs));
}

void
exess_bigint_add(ExessBigint* lhs, const ExessBigint* rhs)
{
  assert(MAX(lhs->n_bigits, rhs->n_bigits) + 1 <= BIGINT_MAX_BIGITS);

  bool     carry = 0;
  unsigned i     = 0;
  for (; i < rhs->n_bigits; ++i) {
    const Hugit sum = (Hugit)lhs->bigits[i] + rhs->bigits[i] + carry;

    lhs->bigits[i] = (Bigit)(sum & bigit_mask);
    carry          = (sum & carry_mask) >> 32U;
  }

  i             = exess_bigint_add_carry(lhs, i, carry);
  lhs->n_bigits = MAX(i, lhs->n_bigits);
  assert(exess_bigint_is_clamped(lhs));
}

static unsigned
exess_bigint_subtract_borrow(ExessBigint* lhs, unsigned i, bool borrow)
{
  for (; borrow; ++i) {
    const Bigit l = lhs->bigits[i];

    lhs->bigits[i] -= borrow;

    borrow = l == 0;
  }

  return i;
}

void
exess_bigint_subtract(ExessBigint* lhs, const ExessBigint* rhs)
{
  assert(exess_bigint_is_clamped(lhs));
  assert(exess_bigint_is_clamped(rhs));
  assert(exess_bigint_compare(lhs, rhs) >= 0);

  bool     borrow = 0;
  unsigned i      = 0;
  for (i = 0; i < rhs->n_bigits; ++i) {
    const Bigit l = lhs->bigits[i];
    const Bigit r = rhs->bigits[i];

    lhs->bigits[i] = l - r - borrow;
    borrow         = l < r || (l == r && borrow);
  }

  exess_bigint_subtract_borrow(lhs, i, borrow);
  exess_bigint_clamp(lhs);
}

static unsigned
exess_bigint_leading_zeros(const ExessBigint* num)
{
  return 32 * (BIGINT_MAX_BIGITS - num->n_bigits) +
         exess_clz32(num->bigits[num->n_bigits - 1]);
}

static Bigit
exess_bigint_left_shifted_bigit_i(const ExessBigint* num,
                                  const Offset       amount,
                                  const unsigned     index)
{
  if (amount.bigits == 0 && amount.bits == 0) {
    return num->bigits[index];
  }

  if (index < amount.bigits) {
    return 0;
  }

  if (amount.bits == 0) { // Simple bigit-aligned shift
    return num->bigits[index - amount.bigits];
  }

  if (index == amount.bigits) { // Last non-zero bigit
    return num->bigits[0] << amount.bits;
  }

  // Bigit + sub-bigit bit offset shift
  const unsigned right_shift = BIGINT_BIGIT_BITS - amount.bits;
  return (num->bigits[index - amount.bigits] << amount.bits) |
         (num->bigits[index - amount.bigits - 1] >> right_shift);
}

Bigit
exess_bigint_left_shifted_bigit(const ExessBigint* num,
                                const unsigned     amount,
                                const unsigned     index)
{
  return exess_bigint_left_shifted_bigit_i(num, make_offset(amount), index);
}

void
exess_bigint_subtract_left_shifted(ExessBigint*       lhs,
                                   const ExessBigint* rhs,
                                   const unsigned     amount)
{
  assert(exess_bigint_is_clamped(lhs));
  assert(exess_bigint_is_clamped(rhs));
#ifndef NDEBUG
  {
    ExessBigint check_rhs = *rhs;
    exess_bigint_shift_left(&check_rhs, amount);
    assert(exess_bigint_compare(lhs, &check_rhs) >= 0);
  }
#endif

  const Offset   offset     = make_offset(amount);
  const unsigned r_n_bigits = rhs->n_bigits + offset.bigits + (bool)offset.bits;

  bool     borrow = 0;
  unsigned i      = 0;
  for (i = 0; i < r_n_bigits; ++i) {
    const Bigit l = lhs->bigits[i];
    const Bigit r = exess_bigint_left_shifted_bigit_i(rhs, offset, i);

    lhs->bigits[i] = l - r - borrow;
    borrow         = l < r || ((l == r) && borrow);
  }

  exess_bigint_subtract_borrow(lhs, i, borrow);
  exess_bigint_clamp(lhs);
}

uint32_t
exess_bigint_divmod(ExessBigint* lhs, const ExessBigint* rhs)
{
  assert(exess_bigint_is_clamped(lhs));
  assert(exess_bigint_is_clamped(rhs));
  assert(rhs->n_bigits > 0);
  if (lhs->n_bigits < rhs->n_bigits) {
    return 0;
  }

  uint32_t       result = 0;
  const Bigit    r0     = rhs->bigits[rhs->n_bigits - 1];
  const unsigned rlz    = exess_bigint_leading_zeros(rhs);

  // Shift and subtract until the LHS does not have more bigits
  while (lhs->n_bigits > rhs->n_bigits) {
    const unsigned llz   = exess_bigint_leading_zeros(lhs);
    const unsigned shift = rlz - llz - 1;

    result += 1U << shift;
    exess_bigint_subtract_left_shifted(lhs, rhs, shift);
  }

  // Handle simple termination cases
  int cmp = exess_bigint_compare(lhs, rhs);
  if (cmp < 0) {
    return result;
  }

  if (cmp > 0 && lhs->n_bigits == 1) {
    assert(rhs->n_bigits == 1);
    const Bigit l0 = lhs->bigits[lhs->n_bigits - 1];

    lhs->bigits[lhs->n_bigits - 1] = l0 % r0;
    lhs->n_bigits -= (lhs->bigits[lhs->n_bigits - 1] == 0);
    return result + l0 / r0;
  }

  // Both now have the same number of digits, finish with subtraction
  for (; cmp >= 0; cmp = exess_bigint_compare(lhs, rhs)) {
    const unsigned llz = exess_bigint_leading_zeros(lhs);
    if (rlz == llz) {
      // Both have the same number of leading zeros, just subtract
      exess_bigint_subtract(lhs, rhs);
      return result + 1;
    }

    const unsigned shift = rlz - llz - 1;
    result += 1U << shift;
    exess_bigint_subtract_left_shifted(lhs, rhs, shift);
  }

  return result;
}
