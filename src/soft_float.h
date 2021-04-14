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

#ifndef EXESS_SOFT_FLOAT_H
#define EXESS_SOFT_FLOAT_H

#include "attributes.h"

#include <stdint.h>

typedef struct {
  uint64_t f; ///< Significand
  int      e; ///< Exponent
} ExessSoftFloat;

static const int min_dec_expt  = -348;
static const int max_dec_expt  = 340;
static const int dec_expt_step = 8;

/// Convert `d` to a soft float
EXESS_I_CONST_FUNC
ExessSoftFloat
soft_float_from_double(double d);

/// Convert `v` to a double
double
soft_float_to_double(ExessSoftFloat v);

/// Normalize `value` so the MSb of its significand is 1
EXESS_I_CONST_FUNC
ExessSoftFloat
soft_float_normalize(ExessSoftFloat value);

/// Multiply `lhs` by `rhs` and return the result
EXESS_I_CONST_FUNC
ExessSoftFloat
soft_float_multiply(ExessSoftFloat lhs, ExessSoftFloat rhs);

/// Return exactly 10^e for e in [0...dec_expt_step]
EXESS_I_CONST_FUNC
ExessSoftFloat
soft_float_exact_pow10(int expt);

/**
   Return a cached power of 10 with exponent not greater than `max_exponent`.

   Valid only for `max_exponent` values from min_dec_expt to max_dec_expt +
   dec_expt_step.  The returned power's exponent is a multiple of
   dec_expt_step.

   @param max_exponent Maximum decimal exponent of the result.
   @param[out] pow10_exponent Set to the decimal exponent of the result.
   @return A cached power of 10 as a soft float.
*/
ExessSoftFloat
soft_float_pow10_under(int max_exponent, int* pow10_exponent);

#endif // EXESS_SOFT_FLOAT_H
