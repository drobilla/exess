// Copyright 2019-2024 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#ifndef EXESS_TEST_FLOAT_TEST_UTILS_H
#define EXESS_TEST_FLOAT_TEST_UTILS_H

#include "ieee_float.h"
#include "warnings.h"

#include <assert.h>
#include <math.h>
#include <stdbool.h>
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

/// Return the distance between two floats in ULPs
static uint32_t
float_ulp_distance(const float a, const float b)
{
  EXESS_DISABLE_CONVERSION_WARNINGS
  assert(!isnan(a));
  assert(!isnan(b));
  EXESS_RESTORE_WARNINGS

  assert(a >= 0.0f);
  assert(b >= 0.0f);

  const uint32_t ia = float_to_rep(a);
  const uint32_t ib = float_to_rep(b);

  return ia >= ib ? ia - ib : ib - ia;
}

static bool
float_matches(const float a, const float b)
{
  EXESS_DISABLE_CONVERSION_WARNINGS
  const bool a_is_nan      = isnan(a);
  const bool a_is_negative = signbit(a);
  const bool b_is_nan      = isnan(b);
  const bool b_is_negative = signbit(b);
  EXESS_RESTORE_WARNINGS

  return (a_is_nan && b_is_nan) ||
         (!a_is_nan && !b_is_nan && a_is_negative == b_is_negative &&
          (a_is_negative ? float_ulp_distance(-a, -b) == 0
                         : float_ulp_distance(a, b) == 0));
}

#endif // EXESS_TEST_FLOAT_TEST_UTILS_H
