// Copyright 2019-2024 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#ifndef EXESS_TEST_DOUBLE_TEST_UTILS_H
#define EXESS_TEST_DOUBLE_TEST_UTILS_H

#include "../src/ieee_float.h"
#include "../src/warnings.h"

#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

/// Return the double with representation `rep`
static inline double
double_from_rep(const uint64_t rep)
{
  double d = 0.0;
  memcpy(&d, &rep, sizeof(d));
  return d;
}

/// Return the distance between two doubles in ULPs
static uint64_t
double_ulp_distance(const double a, const double b)
{
  EXESS_DISABLE_CONVERSION_WARNINGS
  assert(!isnan(a));
  assert(!isnan(b));
  EXESS_RESTORE_WARNINGS

  assert(a >= 0.0);
  assert(b >= 0.0);

  const uint64_t ia = double_to_rep(a);
  const uint64_t ib = double_to_rep(b);

  return ia >= ib ? ia - ib : ib - ia;
}

static bool
double_matches(const double a, const double b)
{
  EXESS_DISABLE_CONVERSION_WARNINGS
  const bool a_is_nan      = isnan(a);
  const bool a_is_negative = signbit(a);
  const bool b_is_nan      = isnan(b);
  const bool b_is_negative = signbit(b);
  EXESS_RESTORE_WARNINGS

  return (a_is_nan && b_is_nan) ||
         (!a_is_nan && !b_is_nan && a_is_negative == b_is_negative &&
          (a_is_negative ? double_ulp_distance(-a, -b) == 0
                         : double_ulp_distance(a, b) == 0));
}

#endif // EXESS_TEST_DOUBLE_TEST_UTILS_H
