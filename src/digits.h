// Copyright 2019-2026 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#ifndef EXESS_SRC_DIGITS_H
#define EXESS_SRC_DIGITS_H

#include "attributes.h"

#include <stddef.h>

typedef struct {
  unsigned count; ///< Number of digits
  int      expt;  ///< Power of 10 exponent
} ExessDigitCount;

/**
   Write significant digits digits for `d` into `digits`.

   Writes only significant digits, without any leading or trailing zeros.  The
   actual number is given by the exponent in the return value.

   @param d The number to convert to digits, must be finite and non-zero.
   @param digits_size The maximum number of digits to write.
   @param digits The output buffer at least `digits_size` long.
*/
EXESS_I_NONBLOCKING ExessDigitCount
generate_digits(double d, size_t digits_size, char* digits);

#endif // EXESS_SRC_DIGITS_H
