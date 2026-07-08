// Copyright 2019-2026 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#include "big_decimal.h"
#include "double_to_decimal.h"
#include "scientific.h"
#include "write_utils.h"

#include <exess/exess.h>

#include <math.h>
#include <string.h>

#ifndef FLT_DECIMAL_DIG
#  define FLT_DECIMAL_DIG 9U
#endif

EXESS_NONBLOCKING ExessResult
exess_read_float(const char* const str, float* const out)
{
  double            value = (double)NAN;
  const ExessResult r     = exess_read_double(str, &value);

  *out = (float)value;

  return r;
}

EXESS_NONBLOCKING ExessResult
exess_write_float(const float value, const size_t buf_size, char* const buf)
{
  char digits[FLT_DECIMAL_DIG + 1U] = {0};

  const BigDecimal decimal =
    double_to_decimal((double)value, FLT_DECIMAL_DIG, digits);

  const ExessResult r = write_scientific(decimal, digits, buf_size, buf);
  return end_write(r.status, buf_size, buf, r.count);
}
