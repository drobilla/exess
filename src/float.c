// Copyright 2019-2021 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#include "floating_decimal.h"
#include "result.h"
#include "scientific.h"

#include <exess/exess.h>

#include <math.h>
#include <string.h>

ExessResult
exess_read_float(float* const out, const char* const str)
{
  double            value = (double)NAN;
  const ExessResult r     = exess_read_double(&value, str);

  *out = (float)value;

  return r;
}

ExessResult
exess_write_float(const float value, const size_t buf_size, char* const buf)
{
  const ExessFloatingDecimal decimal = measure_float(value);

  return buf ? write_scientific(decimal, buf_size, buf)
             : result(EXESS_SUCCESS, scientific_string_length(decimal));
}
