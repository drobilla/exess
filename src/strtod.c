// Copyright 2019-2021 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#include "strtod.h"

#include "attributes.h"
#include "bigint.h"
#include "decimal.h"
#include "ieee_float.h"
#include "int_math.h"
#include "macros.h"
#include "result.h"
#include "soft_float.h"
#include "string_utils.h"

#include <assert.h>
#include <float.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

/// Number of digits always represented exactly by an unsigned 64-bit integer
static const int uint64_digits10 = 19;

static int
read_sign(const char** const sptr)
{
  if (**sptr == '-') {
    ++(*sptr);
    return -1;
  }

  if (**sptr == '+') {
    ++(*sptr);
  }

  return 1;
}

static int
skip_zeros(const char* const str)
{
  int n = 0;
  while (str[n] == '0') {
    ++n;
  }

  return n;
}

ExessResult
parse_decimal(ExessDecimalDouble* const out, const char* const str)
{
  // Read leading sign if present
  const char* s    = str;
  const int   sign = read_sign(&s);

  out->kind = (sign < 0) ? EXESS_NEGATIVE : EXESS_POSITIVE;

  // Check that the first character is valid
  if (*s != '.' && !is_digit(*s)) {
    return result(EXESS_EXPECTED_DIGIT, (size_t)(s - str));
  }

  // Skip leading zeros before decimal point
  const int n_leading_before = skip_zeros(s);
  s += n_leading_before;

  // Skip leading zeros after decimal point
  int  n_leading_after = 0;           // Zeros skipped after decimal point
  bool after_point     = (*s == '.'); // We're after the decimal point
  if (after_point) {
    n_leading_after = skip_zeros(++s);
    s += n_leading_after;
  }

  // Read significant digits of the mantissa into a 64-bit integer
  uint64_t frac    = 0; // Fraction value (ignoring decimal point)
  int      n_after = 0; // Number of digits after decimal point
  for (; out->n_digits < DBL_DECIMAL_DIG + 1; ++s) {
    if (is_digit(*s)) {
      frac = (frac * 10) + (unsigned)(*s - '0');
      n_after += after_point;
      out->digits[out->n_digits++] = *s;
    } else if (*s == '.' && !after_point) {
      after_point = true;
    } else {
      break;
    }
  }

  // Skip extra digits
  int n_extra_before = 0;
  for (;; ++s) {
    if (*s == '.' && !after_point) {
      after_point = true;
    } else if (is_digit(*s)) {
      n_extra_before += !after_point;
    } else {
      break;
    }
  }

  // Calculate final output exponent
  out->expt = n_extra_before - n_after - n_leading_after;

  // Update the kind if necessary to handle zero cases
  out->kind = out->n_digits                   ? out->kind
              : (out->kind == EXESS_NEGATIVE) ? EXESS_NEGATIVE_ZERO
                                              : EXESS_POSITIVE_ZERO;

  return result(EXESS_SUCCESS, (size_t)(s - str));
}

ExessResult
parse_double(ExessDecimalDouble* const out, const char* const str)
{
  // Handle non-numeric special cases

  if (!strcmp(str, "NaN")) {
    out->kind = EXESS_NAN;
    return result(EXESS_SUCCESS, 3U);
  }

  if (!strcmp(str, "-INF")) {
    out->kind = EXESS_NEGATIVE_INFINITY;
    return result(EXESS_SUCCESS, 4U);
  }

  if (!strcmp(str, "INF")) {
    out->kind = EXESS_POSITIVE_INFINITY;
    return result(EXESS_SUCCESS, 3U);
  }

  if (!strcmp(str, "+INF")) {
    out->kind = EXESS_POSITIVE_INFINITY;
    return result(EXESS_SUCCESS, 4U);
  }

  // Read mantissa as a decimal
  const ExessResult r = parse_decimal(out, str);
  if (r.status) {
    return r;
  }

  const char* s = str + r.count;

  // Read exponent
  int abs_expt  = 0;
  int expt_sign = 1;
  if (*s == 'e' || *s == 'E') {
    ++s;

    if (*s != '-' && *s != '+' && !is_digit(*s)) {
      return result(EXESS_EXPECTED_DIGIT, (size_t)(s - str));
    }

    expt_sign = read_sign(&s);
    while (is_digit(*s)) {
      abs_expt = (abs_expt * 10) + (*s++ - '0');
    }
  }

  // Calculate final output exponent
  out->expt += expt_sign * abs_expt;

  if (out->n_digits == 0) {
    out->kind = out->kind < EXESS_POSITIVE_ZERO ? EXESS_NEGATIVE_ZERO
                                                : EXESS_POSITIVE_ZERO;
  }

  return result(EXESS_SUCCESS, (size_t)(s - str));
}

static uint64_t
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
static bool
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
  static const unsigned lg_denom = 3;
  static const uint64_t denom    = 1U << 3U;
  static const uint64_t half_ulp = 4U;

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
  const int      magnitude      = 64 + input.e;
  const int      real_magnitude = magnitude - dbl_subnormal_expt;
  const unsigned n_significant_bits =
    (unsigned)MAX(0, MIN(real_magnitude, DBL_MANT_DIG));

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

static int
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

EXESS_I_PURE_FUNC static uint64_t
read_fraction(size_t n_digits, const char* const digits)
{
  uint64_t frac = 0;

  for (unsigned i = 0U; i < n_digits; ++i) {
    if (is_digit(digits[i])) {
      frac = (frac * 10U) + (unsigned)(digits[i] - '0');
    }
  }

  return frac;
}

double
parsed_double_to_double(const ExessDecimalDouble in)
{
  static const int      n_exact_pow10        = sizeof(POW10) / sizeof(POW10[0]);
  static const unsigned max_exact_int_digits = 15;   // Digits that fit exactly
  static const int      max_decimal_power    = 309;  // Max finite power
  static const int      min_decimal_power    = -324; // Min non-zero power

  // Return early for edge cases
  switch (in.kind) {
  case EXESS_NEGATIVE:
    break;
  case EXESS_NEGATIVE_INFINITY:
    return (double)-INFINITY;
  case EXESS_NEGATIVE_ZERO:
    return -0.0;
  case EXESS_POSITIVE_ZERO:
    return 0.0;
  case EXESS_POSITIVE:
    break;
  case EXESS_POSITIVE_INFINITY:
    return (double)INFINITY;
  case EXESS_NAN:
    return (double)NAN;
  }

  const uint64_t frac         = read_fraction(in.n_digits, in.digits);
  const int      sign         = in.kind >= EXESS_POSITIVE_ZERO ? 1 : -1;
  const int      result_power = (int)in.n_digits + in.expt;

  // Return early for simple exact cases

  if (result_power > max_decimal_power) {
    return sign * (double)INFINITY;
  }

  if (result_power < min_decimal_power) {
    return sign * 0.0;
  }

  if (in.n_digits < max_exact_int_digits) {
    if (in.expt < 0 && -in.expt < n_exact_pow10) {
      return sign * ((double)frac / (double)POW10[-in.expt]);
    }

    if (in.expt >= 0 && in.expt < n_exact_pow10) {
      return sign * ((double)frac * (double)POW10[in.expt]);
    }
  }

  // Try to guess the number using only soft floating point (fast path)
  ExessSoftFloat guess = {0, 0};
  const bool     exact = sftod(frac, in.expt, (int)in.n_digits, &guess);
  const double   g     = soft_float_to_double(guess);
  if (exact) {
    return sign * g;
  }

  // Not sure, guess is either the number or its predecessor (rare slow path)
  // Compare it with the buffer using bigints to find out which
  const ExessSoftFloat upper = {guess.f * 2 + 1, guess.e - 1};
  const int            cmp   = compare_buffer(in.digits, in.expt, upper);
  const bool round_up        = (cmp > 0) || (cmp == 0 && (guess.f & 1U) != 0);

  return sign * (round_up ? nextafter(g, (double)INFINITY) : g);
}
