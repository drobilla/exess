// Copyright 2019-2024 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#include "scientific.h"
#include "floating_decimal.h"
#include "int_math.h"
#include "result.h"
#include "write_utils.h"

#include <exess/exess.h>

#include <stdlib.h>
#include <string.h>

typedef struct {
  size_t      length;
  const char* string;
} SpecialCase;

static const SpecialCase special_cases[] = {{3U, "NaN"},
                                            {4U, "-INF"},
                                            {3U, "INF"},
                                            {6U, "-0.0E0"},
                                            {5U, "0.0E0"}};

size_t
scientific_string_length(const ExessFloatingDecimal value)
{
  if (value.kind < EXESS_NEGATIVE) {
    return special_cases[value.kind].length;
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
write_scientific(const ExessFloatingDecimal value,
                 const size_t               n,
                 char* const                buf)
{
  size_t i = 0;

  if (n <= (value.kind == EXESS_NEGATIVE) + value.n_digits + 1) {
    buf[0] = '\0';
    return result(EXESS_NO_SPACE, 0);
  }

  if (value.kind < EXESS_NEGATIVE) {
    return write_special(special_cases[value.kind].length,
                         special_cases[value.kind].string,
                         n,
                         buf);
  }

  if (value.kind == EXESS_NEGATIVE) {
    buf[i++] = '-';
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

  if (n - i <= 1U + (value.expt < 0) + n_expt_digits) {
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
