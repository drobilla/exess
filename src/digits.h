// Copyright 2019-2021 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#ifndef EXESS_SRC_DIGITS_H
#define EXESS_SRC_DIGITS_H

typedef struct {
  unsigned count; ///< Number of digits
  int      expt;  ///< Power of 10 exponent
} ExessDigitCount;

/**
   Write significant digits digits for `d` into `buf`.

   Writes only significant digits, without any leading or trailing zeros.  The
   actual number is given by the exponent in the return value.

   @param d The number to convert to digits, must be finite and non-zero.
   @param max_digits The maximum number of digits to write.
   @param buf The output buffer at least `max_digits` long.
*/
ExessDigitCount
generate_digits(double d, unsigned max_digits, char* buf);

#endif // EXESS_SRC_DIGITS_H
