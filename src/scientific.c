// Copyright 2019-2025 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#include "scientific.h"
#include "big_decimal.h"
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

EXESS_CONST_FUNC static size_t
scientific_string_length(const BigDecimal value, const unsigned n_expt_digits)
{
  if (value.kind < EXESS_NEGATIVE) {
    return special_cases[value.kind].length;
  }

  return ((value.kind == EXESS_NEGATIVE) + // Sign
          value.n_digits + 1 +             // Digits and point
          (value.n_digits <= 1) +          // Added '0' after point
          1 +                              // 'E'
          (value.expt < 1) +               // Exponent sign
          n_expt_digits);                  // Exponent digits
}

ExessResult
write_scientific(const BigDecimal value, const size_t buf_size, char* const buf)
{
  const int      expt          = value.expt - 1;
  unsigned       abs_expt      = (unsigned)abs(expt);
  const unsigned n_expt_digits = (unsigned)exess_num_digits(abs_expt);
  const size_t   length        = scientific_string_length(value, n_expt_digits);
  if (length >= buf_size) {
    return RESULT(EXESS_NO_SPACE, length);
  }

  if (value.kind < EXESS_NEGATIVE) {
    return write_special(special_cases[value.kind].length,
                         special_cases[value.kind].string,
                         buf_size,
                         buf);
  }

  size_t i = 0;
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

  buf[i++] = 'E';
  if (expt < 0) {
    buf[i++] = '-';
  }

  char* s = buf + i + n_expt_digits;

  *s-- = '\0';
  do {
    *s-- = (char)('0' + (abs_expt % 10));
  } while ((abs_expt /= 10) > 0);

  return RESULT(EXESS_SUCCESS, i + n_expt_digits);
}
