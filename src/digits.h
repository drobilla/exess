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

#ifndef EXESS_DIGITS_H
#define EXESS_DIGITS_H

typedef struct {
  unsigned count; ///< Number of digits
  int      expt;  ///< Power of 10 exponent
} ExessDigitCount;

/**
   Write significant digits digits for `d` into `buf`.

   Writes only significant digits, without any leading or trailing zeros.  The
   actual number is given by the exponent in the return value.

   @param d The number to convert to digits, must be finite and non-zero.
   @param buf The output buffer at least `max_digits` long.
   @param max_digits The maximum number of digits to write.
*/
ExessDigitCount
exess_digits(double d, char* buf, unsigned max_digits);

#endif // EXESS_DIGITS_H
