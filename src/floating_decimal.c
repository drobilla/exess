// Copyright 2019-2024 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#include "floating_decimal.h"
#include "digits.h"
#include "warnings.h"

#include <assert.h>
#include <math.h>
#include <stdbool.h>

typedef enum {
  EXESS_POINT_AFTER,   ///< Decimal point is after all significant digits
  EXESS_POINT_BEFORE,  ///< Decimal point is before all significant digits
  EXESS_POINT_BETWEEN, ///< Decimal point is between significant digits
} ExessPointLocation;

static ExessNumberKind
number_kind(const double d)
{
  EXESS_DISABLE_CONVERSION_WARNINGS
  const int  fpclass     = fpclassify(d);
  const bool is_negative = signbit(d);
  EXESS_RESTORE_WARNINGS

  switch (fpclass) {
  case FP_ZERO:
    return is_negative ? EXESS_NEGATIVE_ZERO : EXESS_POSITIVE_ZERO;
  case FP_INFINITE:
    return is_negative ? EXESS_NEGATIVE_INFINITY : EXESS_POSITIVE_INFINITY;
  case FP_NORMAL:
  case FP_SUBNORMAL:
    return is_negative ? EXESS_NEGATIVE : EXESS_POSITIVE;
  default:
    break;
  }

  return EXESS_NAN;
}

static ExessFloatingDecimal
measure_decimal(const double d, const unsigned max_precision)
{
  ExessFloatingDecimal value = {number_kind(d), 0, 0, {0}};

  if (value.kind != EXESS_NEGATIVE && value.kind != EXESS_POSITIVE) {
    return value;
  }

  // Get decimal digits
  const ExessDigitCount count =
    generate_digits(fabs(d), max_precision, value.digits);

  assert(count.count == 1 || value.digits[count.count - 1] != '0');

  value.n_digits = count.count;
  value.expt     = count.expt;

  return value;
}

ExessFloatingDecimal
measure_float(const float f)
{
  return measure_decimal((double)f, FLT_DECIMAL_DIG);
}

ExessFloatingDecimal
measure_double(const double d)
{
  return measure_decimal(d, DBL_DECIMAL_DIG);
}
