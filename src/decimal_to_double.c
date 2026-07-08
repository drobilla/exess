// Copyright 2019-2026 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#include <exess/exess.h>

#include "big_decimal.h"
#include "bigint.h"
#include "decimal_to_double.h"
#include "ieee_float.h"
#include "int_math.h"
#include "macros.h"
#include "soft_float.h"

#include <assert.h>
#include <float.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

/// Number of digits always represented exactly by an unsigned 64-bit integer
static const int uint64_digits10 = 19;

static EXESS_NONBLOCKING uint64_t
normalize(ExessSoftFloat* value, const uint64_t error)
{
  const int original_e = value->e;

  *value = soft_float_normalize(*value);

  assert(value->e <= original_e);
  return error << (unsigned)(original_e - value->e);
}

/**
   Return the error added by floating point multiplication.

   Should be l + r + l*r/(2^64) + 0.5, but we short the denominator to 63 due
   to lack of precision, which effectively rounds up.
*/
static inline uint64_t
product_error(const uint64_t lerror,
              const uint64_t rerror,
              const uint64_t half_ulp)
{
  return lerror + rerror + ((lerror * rerror) >> 63U) + half_ulp;
}

/**
   Guess the binary floating point value for decimal input.

   @param significand Significand from the input.
   @param expt10 Decimal exponent from the input.
   @param n_digits Number of decimal digits in the significand.
   @param[out] guess Either the exact number, or its predecessor.
   @return True if `guess` is correct.
*/
static EXESS_I_NONBLOCKING bool
sftod(const uint64_t        significand,
      const int             expt10,
      const int             n_digits,
      ExessSoftFloat* const guess)
{
  assert(expt10 <= max_dec_expt);
  assert(expt10 >= min_dec_expt);

  /* The general idea here is to try and find a power of 10 that we can
     multiply by the significand to get the number.  We get one from the
     cache which is possibly too small, then multiply by another power of 10
     to make up the difference if necessary.  For example, with a target
     power of 10^70, if we get 10^68 from the cache, then we multiply again
     by 10^2.  This, as well as normalization, accumulates error, which is
     tracked throughout to know if we got the precise number. */

  // Use a common denominator of 2^3 to avoid fractions
  EXESS_CONSTEXPR unsigned lg_denom = 3;
  EXESS_CONSTEXPR uint64_t denom    = 1U << 3U;
  EXESS_CONSTEXPR uint64_t half_ulp = 4U;

  // Start out with just the significand, and no error
  ExessSoftFloat input = {significand, 0};
  uint64_t       error = normalize(&input, 0);

  // Get a power of 10 that takes us most of the way without overshooting
  int            cached_expt10 = 0;
  ExessSoftFloat pow10         = soft_float_pow10_under(expt10, &cached_expt10);

  // Get an exact fixup power if necessary
  const int d_expt10 = expt10 - cached_expt10;
  if (d_expt10) {
    input = soft_float_multiply(input, soft_float_exact_pow10(d_expt10));
    if (d_expt10 > uint64_digits10 - n_digits) {
      error += half_ulp; // Product does not fit in an integer
    }
  }

  // Multiply the significand by the power, normalize, and update the error
  input = soft_float_multiply(input, pow10);
  error = normalize(&input, product_error(error, half_ulp, half_ulp));

  // Get the effective number of significant bits from the order of magnitude
  const int magnitude = MIN(DBL_MANT_DIG, 64 + input.e - dbl_subnormal_expt);
  const unsigned n_significant_bits = (unsigned)MAX(0, magnitude);

  // Calculate the number of "extra" bits of precision we have
  assert(n_significant_bits <= 64);
  unsigned n_extra_bits = 64U - n_significant_bits;
  if (n_extra_bits + lg_denom >= 64U) {
    // Very small subnormal where extra * denom does not fit in an integer
    // Shift right (and accumulate some more error) to compensate
    const unsigned amount = (n_extra_bits + lg_denom) - 63;

    input.f >>= amount;
    input.e += (int)amount;
    error = product_error((error >> amount) + 1U, half_ulp, half_ulp);
    n_extra_bits -= amount;
  }

  // Calculate boundaries for the extra bits (with the common denominator)
  assert(n_extra_bits < 64);
  const uint64_t extra_mask = (1ULL << n_extra_bits) - 1U;
  const uint64_t extra_bits = (input.f & extra_mask) * denom;
  const uint64_t middle     = (1ULL << (n_extra_bits - 1U)) * denom;
  const uint64_t low        = middle - error;
  const uint64_t high       = middle + error;

  // Round to nearest representable double
  guess->f = (input.f >> n_extra_bits) + (extra_bits >= high);
  guess->e = input.e + (int)n_extra_bits;

  // Too inaccurate if the extra bits are within the error around the middle
  return extra_bits <= low || extra_bits >= high;
}

static EXESS_I_NONBLOCKING int
compare_buffer(const char* buf, const int expt, const ExessSoftFloat upper)
{
  ExessBigint buf_bigint;
  exess_bigint_set_decimal_string(&buf_bigint, buf);

  ExessBigint upper_bigint;
  exess_bigint_set_u64(&upper_bigint, upper.f);

  if (expt >= 0) {
    exess_bigint_multiply_pow10(&buf_bigint, (unsigned)expt);
  } else {
    exess_bigint_multiply_pow10(&upper_bigint, (unsigned)-expt);
  }

  if (upper.e > 0) {
    exess_bigint_shift_left(&upper_bigint, (unsigned)upper.e);
  } else {
    exess_bigint_shift_left(&buf_bigint, (unsigned)-upper.e);
  }

  return exess_bigint_compare(&buf_bigint, &upper_bigint);
}

EXESS_PURE_FUNC static uint64_t
read_fraction(size_t n_digits, const char* const digits)
{
  uint64_t frac = 0;

  for (unsigned i = 0U; i < n_digits; ++i) {
    assert(digits[i] >= '0' && digits[i] <= '9');
    frac = (frac * 10U) + (unsigned)(digits[i] - '0');
  }

  return frac;
}

EXESS_I_NONBLOCKING double
decimal_to_double(const BigDecimal in, const char* const digits)
{
  EXESS_CONSTEXPR int n_exact_pow10 = sizeof(POW10) / sizeof(POW10[0]);

  EXESS_CONSTEXPR unsigned max_exact_int_digits = 15; // Digits that fit exactly
  EXESS_CONSTEXPR int      max_decimal_power    = 309;  // Max finite power
  EXESS_CONSTEXPR int      min_decimal_power    = -324; // Min non-zero power

  EXESS_CONSTEXPR double special_values[] = {
    (double)NAN, (double)-INFINITY, (double)INFINITY, -0.0, 0.0};

  if (in.kind < EXESS_NEGATIVE) {
    return special_values[in.kind]; // Special case
  }

  const int sign = in.kind == EXESS_POSITIVE ? 1 : -1;

  if (in.expt > max_decimal_power) {
    return sign * (double)INFINITY; // Infinite magnitude => infinity
  }

  if (in.expt < min_decimal_power) {
    return sign * 0.0; // Effectively zero
  }

  // Represent value with integer mantissa and adjusted decimal power
  const uint64_t frac  = read_fraction(in.n_digits, digits);
  const int      power = in.expt - (int)in.n_digits;

  if (in.n_digits < max_exact_int_digits) { // Exact integer value
    if (power < 0) {
      if (-power < n_exact_pow10) {
        return sign * ((double)frac / (double)POW10[-power]);
      }
    } else if (power < n_exact_pow10) {
      return sign * ((double)frac * (double)POW10[power]);
    }
  }

  // Try to guess the number using only soft floating point (fast path)
  ExessSoftFloat guess = {0, 0};
  const bool     exact = sftod(frac, power, (int)in.n_digits, &guess);
  const double   g     = soft_float_to_double(guess);
  if (exact) {
    return sign * g;
  }

  // Not sure, guess is either the number or its predecessor (rare slow path)
  // Compare it with the buffer using bigints to find out which
  const ExessSoftFloat upper = {(guess.f * 2) + 1, guess.e - 1};
  const int            cmp   = compare_buffer(digits, power, upper);
  const bool round_up        = (cmp > 0) || (cmp == 0 && (guess.f & 1U) != 0);

  return sign * (round_up ? nextafter(g, (double)INFINITY) : g);
}
