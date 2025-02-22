// Copyright 2019-2025 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#include "digits.h"

#include "bigint.h"
#include "ieee_float.h"
#include "int_math.h"
#include "soft_float.h"
#include "warnings.h"

#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/*
  This is more or less just an implementation of the classic rational number
  based floating point print routine ("Dragon4").  See "How to Print
  Floating-Point Numbers Accurately" by Guy L. Steele Jr. and Jon L White for
  the canonical source.  The basic idea is to find a big rational between 1 and
  10 where value = (numer / denom) * 10^e, then continuously divide it to
  generate decimal digits.

  Unfortunately, this algorithm requires pretty massive bigints to work
  correctly for all doubles, and isn't particularly fast.  Something like
  Grisu3 could be added to improve performance, but that has the annoying
  property of needing a more precise fallback in some cases, meaning it would
  only add more code, not replace any.  Since this is already a pretty
  ridiculous amount of code, I'll hold off on this until it becomes a problem,
  or somebody comes up with a better algorithm.
*/

/// Return true if the number is within the lower boundary
static bool
within_lower(const ExessBigint* const numer,
             const ExessBigint* const d_lower,
             const bool               is_even)
{
  return is_even ? exess_bigint_compare(numer, d_lower) <= 0
                 : exess_bigint_compare(numer, d_lower) < 0;
}

/// Return true if the number is within the upper boundary
static bool
within_upper(const ExessBigint* const numer,
             const ExessBigint* const denom,
             const ExessBigint* const d_upper,
             const bool               is_even)
{
  return is_even ? exess_bigint_plus_compare(numer, d_upper, denom) >= 0
                 : exess_bigint_plus_compare(numer, d_upper, denom) > 0;
}

/**
   Find values so that 0.1 <= numer/denom < 1 or 1 <= numer/denom < 10.

   @param significand Double significand.
   @param exponent Double exponent (base 2).
   @param decimal_power Decimal exponent (log10 of the double).
   @param[out] numer Numerator of rational number.
   @param[out] denom Denominator of rational number.
   @param[out] delta Distance to the lower and upper boundaries.
*/
static void
calculate_initial_values(const uint64_t     significand,
                         const int          exponent,
                         const int          decimal_power,
                         const bool         lower_is_closer,
                         ExessBigint* const numer,
                         ExessBigint* const denom,
                         ExessBigint* const delta)
{
  /* Use a common denominator of 2^1 so that boundary distance is an integer.
     If the lower boundary is closer, we need to scale everything but the
     lower boundary to compensate, so add another factor of two here (this is
     faster than shifting them again later as in the paper). */
  const unsigned lg_denom = 1U + lower_is_closer;

  if (exponent >= 0) {
    // delta = 2^e
    exess_bigint_set_u32(delta, 1);
    exess_bigint_shift_left(delta, (unsigned)exponent);

    // numer = f * 2^e
    exess_bigint_set_u64(numer, significand);
    exess_bigint_shift_left(numer, (unsigned)exponent + lg_denom);

    // denom = 10^d
    exess_bigint_set_pow10(denom, (unsigned)decimal_power);
    exess_bigint_shift_left(denom, lg_denom);
  } else if (decimal_power >= 0) {
    // delta = 2^e, which is just 1 here since 2^-e is in the denominator
    exess_bigint_set_u32(delta, 1);

    // numer = f
    exess_bigint_set_u64(numer, significand);
    exess_bigint_shift_left(numer, lg_denom);

    // denom = 10^d * 2^-e
    exess_bigint_set_pow10(denom, (unsigned)decimal_power);
    exess_bigint_shift_left(denom, (unsigned)-exponent + lg_denom);
  } else {
    // delta = 10^d
    exess_bigint_set_pow10(delta, (unsigned)-decimal_power);

    // numer = f * 10^-d
    exess_bigint_set(numer, delta);
    exess_bigint_multiply_u64(numer, significand);
    exess_bigint_shift_left(numer, lg_denom);

    // denom = 2^-exponent
    exess_bigint_set_u32(denom, 1);
    exess_bigint_shift_left(denom, (unsigned)-exponent + lg_denom);
  }
}

#ifndef NDEBUG
static bool
check_initial_values(const ExessBigint* const numer,
                     const ExessBigint* const denom,
                     const ExessBigint* const d_upper)
{
  ExessBigint upper = *numer;
  exess_bigint_add(&upper, d_upper);
  assert(exess_bigint_compare(&upper, denom) >= 0);

  const uint32_t div = exess_bigint_divmod(&upper, denom);
  assert(div >= 1 && div < 10);
  return true;
}
#endif

static unsigned
emit_digits(ExessBigint* const       numer,
            const ExessBigint* const denom,
            ExessBigint* const       d_lower,
            ExessBigint* const       d_upper,
            const bool               is_even,
            const unsigned           max_digits,
            char* const              buffer)
{
  unsigned length = 0;
  for (size_t i = 0; i < max_digits; ++i) {
    // Emit the next digit
    const uint32_t digit = exess_bigint_divmod(numer, denom);
    assert(digit <= 9);
    buffer[length++] = (char)('0' + digit);

    // Check for termination
    const bool within_low  = within_lower(numer, d_lower, is_even);
    const bool within_high = within_upper(numer, denom, d_upper, is_even);
    if (!within_low && !within_high) {
      exess_bigint_multiply_u32(numer, 10);
      exess_bigint_multiply_u32(d_lower, 10);
      if (d_lower != d_upper) {
        exess_bigint_multiply_u32(d_upper, 10);
      }
    } else {
      if (!within_low || (within_high && exess_bigint_plus_compare(
                                           numer, numer, denom) >= 0)) {
        // In high only, or halfway and the next digit is > 5, round up
        assert(buffer[length - 1] != '9');
        buffer[length - 1]++;
      }

      break;
    }
  }

  return length;
}

/// Return true if the lower boundary is closer than the upper boundary
static bool
double_lower_boundary_is_closer(const double d)
{
  const uint64_t rep          = double_to_rep(d);
  const uint64_t mant         = rep & dbl_mant_mask;
  const uint64_t expt         = rep & dbl_expt_mask;
  const bool     is_subnormal = expt == 0;

  // True when f = 2^(p-1) (except for the smallest normal)
  return !is_subnormal && mant == 0;
}

/// Return an estimate for the decimal power of value, undershot by at most 1
static int
approximate_power(const ExessSoftFloat value)
{
  /* See "Printing Floating-Point Numbers Quickly and Accurately" by Robert
     G. Burger and R. Kent Dybvig.  The trick of undershooting by 0.69 comes
     from the implementation by Ryan Juckett, see
     https://ryanjuckett.com/printing-floating-point-numbers-part-2-dragon4/
  */

  static const double log10_2 = 0.30102999566398119521373889472449;

  const double f_msb_index = 64U - exess_clz64(value.f);
  const double power       = ceil(((f_msb_index + value.e) * log10_2) - 0.69);

  return (int)power;
}

ExessDigitCount
generate_digits(const double d, const unsigned max_digits, char* const buf)
{
  EXESS_DISABLE_CONVERSION_WARNINGS
  assert(isfinite(d) && fpclassify(d) != FP_ZERO);
  EXESS_RESTORE_WARNINGS

  const ExessSoftFloat value           = soft_float_from_double(d);
  const int            power           = approximate_power(value);
  const bool           is_even         = !(value.f & 1U);
  const bool           lower_is_closer = double_lower_boundary_is_closer(d);

  // Calculate initial values so that v = (numer / denom) * 10^power
  ExessBigint numer;
  ExessBigint denom;
  ExessBigint d_lower;
  calculate_initial_values(
    value.f, value.e, power, lower_is_closer, &numer, &denom, &d_lower);

  ExessBigint  d_upper_storage;
  ExessBigint* d_upper = NULL;
  if (lower_is_closer) {
    // Scale upper boundary to account for the closer lower boundary
    // (the numerator and denominator were already scaled above)
    d_upper_storage = d_lower;
    d_upper         = &d_upper_storage;
    exess_bigint_shift_left(d_upper, 1);
  } else {
    d_upper = &d_lower; // Boundaries are the same, reuse the lower
  }

  // Scale if necessary to make 1 <= (numer + delta) / denom < 10
  ExessDigitCount count = {0, 0};
  if (within_upper(&numer, &denom, d_upper, is_even)) {
    count.expt = power;
  } else {
    count.expt = power - 1;
    exess_bigint_multiply_u32(&numer, 10);
    exess_bigint_multiply_u32(&d_lower, 10);
    if (d_upper != &d_lower) {
      exess_bigint_multiply_u32(d_upper, 10);
    }
  }

  // Write digits to output
  assert(check_initial_values(&numer, &denom, d_upper));
  count.count =
    emit_digits(&numer, &denom, &d_lower, d_upper, is_even, max_digits, buf);

  // Trim trailing zeros
  while (count.count > 1 && buf[count.count - 1] == '0') {
    buf[--count.count] = 0;
  }

  buf[count.count] = '\0';
  return count;
}
