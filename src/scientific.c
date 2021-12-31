// Copyright 2019-2021 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#include "scientific.h"
#include "decimal.h"
#include "int_math.h"
#include "result.h"
#include "write_utils.h"

#include "exess/exess.h"

#include <stdlib.h>
#include <string.h>

size_t
exess_scientific_string_length(const ExessDecimalDouble value)
{
  switch (value.kind) {
  case EXESS_NEGATIVE:
    break;
  case EXESS_NEGATIVE_INFINITY:
    return 4;
  case EXESS_NEGATIVE_ZERO:
    return 6;
  case EXESS_POSITIVE_ZERO:
    return 5;
  case EXESS_POSITIVE:
    break;
  case EXESS_POSITIVE_INFINITY:
  case EXESS_NAN:
    return 3;
  }

  const unsigned n_expt_digits =
    (unsigned)exess_num_digits((unsigned)abs(value.expt));

  return ((value.kind == EXESS_NEGATIVE) + // Sign
          value.n_digits + 1 +             // Digits and point
          (value.n_digits <= 1) +          // Added '0' after point
          1 +                              // 'E'
          (value.expt < 0) +               // Exponent sign
          n_expt_digits);                  // Exponent digits
}

ExessResult
exess_write_scientific(const ExessDecimalDouble value,
                       const size_t             n,
                       char* const              buf)
{
  size_t i = 0;

  if (n < 4) {
    return result(EXESS_NO_SPACE, 0);
  }

  switch (value.kind) {
  case EXESS_NEGATIVE:
    buf[i++] = '-';
    break;
  case EXESS_NEGATIVE_INFINITY:
    return write_special(4, "-INF", n, buf);
  case EXESS_NEGATIVE_ZERO:
    return write_special(6, "-0.0E0", n, buf);
  case EXESS_POSITIVE_ZERO:
    return write_special(5, "0.0E0", n, buf);
  case EXESS_POSITIVE:
    break;
  case EXESS_POSITIVE_INFINITY:
    return write_special(3, "INF", n, buf);
  case EXESS_NAN:
    return write_special(3, "NaN", n, buf);
  }

  if (n - i <= value.n_digits + 1) {
    buf[0] = '\0';
    return result(EXESS_NO_SPACE, 0);
  }

  // Write mantissa, with decimal point after the first (normal form)
  buf[i++] = value.digits[0];
  buf[i++] = '.';
  if (value.n_digits > 1) {
    memcpy(buf + i, value.digits + 1, value.n_digits - 1);
    i += value.n_digits - 1;
  } else {
    buf[i++] = '0';
  }

  // Write exponent

  const unsigned n_expt_digits = exess_num_digits((unsigned)abs(value.expt));

  if (n - i <= 1u + (value.expt < 0) + n_expt_digits) {
    buf[0] = '\0';
    return result(EXESS_NO_SPACE, 0);
  }

  buf[i++] = 'E';
  if (value.expt < 0) {
    buf[i++] = '-';
  }

  unsigned abs_expt = (unsigned)abs(value.expt);
  char*    s        = buf + i + n_expt_digits;

  *s-- = '\0';
  do {
    *s-- = (char)('0' + (abs_expt % 10));
  } while ((abs_expt /= 10) > 0);

  return result(EXESS_SUCCESS, i + n_expt_digits);
}
