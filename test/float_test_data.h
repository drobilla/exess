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

#include "attributes.h"
#include "ieee_float.h"
#include "warnings.h"

#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <string.h>

/// Return the float with representation `rep`
static inline float
float_from_rep(const uint32_t rep)
{
  float f = 0.0f;
  memcpy(&f, &rep, sizeof(f));
  return f;
}

/// Return the double with representation `rep`
static inline double
double_from_rep(const uint64_t rep)
{
  double d = 0.0;
  memcpy(&d, &rep, sizeof(d));
  return d;
}

/// Return the distance between two doubles in ULPs
static EXESS_I_PURE_FUNC uint64_t
double_ulp_distance(const double a, const double b)
{
  assert(a >= 0.0);
  assert(b >= 0.0);

  const uint64_t ia = double_to_rep(a);
  const uint64_t ib = double_to_rep(b);
  if (ia == ib) {
    return 0;
  }

  EXESS_DISABLE_CONVERSION_WARNINGS
  if (isnan(a) || isnan(b) || isinf(a) || isinf(b)) {
    return UINT64_MAX;
  }
  EXESS_RESTORE_WARNINGS

  return ia > ib ? ia - ib : ib - ia;
}

/// Return the distance between two floats in ULPs
static EXESS_I_PURE_FUNC uint32_t
float_ulp_distance(const float a, const float b)
{
  assert(a >= 0.0f);
  assert(b >= 0.0f);

  const uint32_t ia = float_to_rep(a);
  const uint32_t ib = float_to_rep(b);
  if (ia == ib) {
    return 0;
  }

  EXESS_DISABLE_CONVERSION_WARNINGS
  if (isnan(a) || isnan(b) || isinf(a) || isinf(b)) {
    return UINT32_MAX;
  }
  EXESS_RESTORE_WARNINGS

  return ia > ib ? ia - ib : ib - ia;
}

static inline bool
float_matches(const float a, const float b)
{
  EXESS_DISABLE_CONVERSION_WARNINGS
  const bool a_is_nan      = isnan(a);
  const bool a_is_negative = signbit(a);
  const bool b_is_nan      = isnan(b);
  const bool b_is_negative = signbit(b);
  EXESS_RESTORE_WARNINGS

  if (a_is_nan && b_is_nan) {
    return true;
  }

  if (a_is_nan || b_is_nan || a_is_negative != b_is_negative) {
    return false;
  }

  return a_is_negative ? float_ulp_distance(-a, -b) == 0
                       : float_ulp_distance(a, b) == 0;
}

static inline bool
double_matches(const double a, const double b)
{
  EXESS_DISABLE_CONVERSION_WARNINGS
  const bool a_is_nan      = isnan(a);
  const bool a_is_negative = signbit(a);
  const bool b_is_nan      = isnan(b);
  const bool b_is_negative = signbit(b);
  EXESS_RESTORE_WARNINGS

  if (a_is_nan && b_is_nan) {
    return true;
  }

  if (a_is_nan || b_is_nan || a_is_negative != b_is_negative) {
    return false;
  }

  return a_is_negative ? double_ulp_distance(-a, -b) == 0
                       : double_ulp_distance(a, b) == 0;
}
