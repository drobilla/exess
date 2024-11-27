// Copyright 2019-2021 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#include "floating_decimal.h"
#include "read_utils.h"
#include "result.h"
#include "scientific.h"
#include "strtod.h"
#include "write_utils.h"

#include <exess/exess.h>

#include <math.h>
#include <string.h>

ExessResult
exess_read_double(double* const out, const char* const str)
{
  *out = (double)NAN;

  const size_t         i  = skip_whitespace(str);
  ExessFloatingDecimal in = {EXESS_NAN, 0U, 0, {0}};
  const ExessResult    r  = parse_double(&in, str + i);

  if (!r.status) {
    *out = decimal_to_double(in);
  }

  return result(r.status, i + r.count);
}

ExessResult
exess_write_double(const double value, const size_t buf_size, char* const buf)
{
  const ExessFloatingDecimal decimal = measure_double(value);

  const ExessResult r =
    buf ? write_scientific(decimal, buf_size, buf)
        : result(EXESS_SUCCESS, scientific_string_length(decimal));

  return end_write(r.status, buf_size, buf, r.count);
}
