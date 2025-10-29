// Copyright 2019-2021 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#include "floating_decimal.h"
#include "scientific.h"

#include <exess/exess.h>

#include <math.h>
#include <string.h>

ExessResult
exess_read_float(const char* const str, float* const out)
{
  double            value = (double)NAN;
  const ExessResult r     = exess_read_double(str, &value);

  *out = (float)value;

  return r;
}

ExessResult
exess_write_float(const float value, const size_t buf_size, char* const buf)
{
  const ExessFloatingDecimal decimal = measure_float(value);

  return write_scientific(decimal, buf_size, buf);
}
