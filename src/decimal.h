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

#ifndef EXESS_DECIMAL_H
#define EXESS_DECIMAL_H

#include "exess/exess.h"

#include <stddef.h>

// Define C11 numeric constants if the compiler hasn't already
#ifndef FLT_DECIMAL_DIG
#  define FLT_DECIMAL_DIG 9
#endif
#ifndef DBL_DECIMAL_DIG
#  define DBL_DECIMAL_DIG 17
#endif

typedef enum {
  EXESS_NEGATIVE,
  EXESS_NEGATIVE_INFINITY,
  EXESS_NEGATIVE_ZERO,
  EXESS_POSITIVE_ZERO,
  EXESS_POSITIVE,
  EXESS_POSITIVE_INFINITY,
  EXESS_NAN,
} ExessNumberKind;

typedef struct {
  ExessNumberKind kind;                        ///< Kind of number
  int             expt;                        ///< Power of 10 exponent
  unsigned        n_digits;                    ///< Number of significant digits
  char            digits[DBL_DECIMAL_DIG + 2]; ///< Significant digits
} ExessDecimalDouble;

ExessDecimalDouble
exess_measure_decimal(double d, unsigned max_precision);

ExessDecimalDouble
exess_measure_float(float f);

ExessDecimalDouble
exess_measure_double(double d);

ExessResult
exess_write_decimal_double(ExessDecimalDouble decimal,
                           size_t             buf_size,
                           char*              buf);

#endif // EXESS_DECIMAL_H
