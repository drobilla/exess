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

#include "decimal.h"
#include "read_utils.h"
#include "scientific.h"

#include "exess/exess.h"

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
  const ExessDecimalDouble decimal = exess_measure_float(value);

  return buf ? exess_write_scientific(decimal, buf_size, buf)
             : result(EXESS_SUCCESS, exess_scientific_string_length(decimal));
}
