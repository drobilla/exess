// Copyright 2019-2025 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#include "parse_decimal.h"

#include "big_decimal.h"
#include "char_utils.h"
#include "macros.h"
#include "number_utils.h"
#include "result.h"

#include <exess/exess.h>

#include <stdbool.h>
#include <stdint.h>

#define BIG_DECIMAL_MAX_DIGITS 255U

typedef struct {
  int  expt;        // Running power of 10 exponent
  bool after_point; // True if we're past the decimal point
} ParseState;

static size_t
skip_leading_zeros(const char* const str, ParseState* const out)
{
  out->after_point = false;

  for (size_t i = 0;; ++i) {
    if (str[i] == '.' && !out->after_point) {
      out->after_point = true;
    } else if (str[i] == '0') {
      out->expt -= out->after_point;
    } else {
      return i;
    }
  }
}

ExessResult
parse_decimal(const char* const str,
              BigDecimal* const out,
              const size_t      digits_size,
              char* const       digits)
{
  // Read leading sign if present
  int    sign = 0;
  size_t i    = read_sign(&sign, str);

  // Skip any leading zeros (before and after decimal point)
  ParseState state = {0, false};
  i += skip_leading_zeros(&str[i], &state);

  // Check that the first character is valid
  if (!state.after_point && !is_digit(str[i])) {
    return RESULT(EXESS_EXPECTED_DIGIT, i);
  }

  // Read significant digits of the mantissa
  const size_t max_n_digits = MIN(digits_size, BIG_DECIMAL_MAX_DIGITS);
  bool         approximate  = false;
  for (;; ++i) {
    if (is_digit(str[i])) {
      const bool significant = (out->n_digits < max_n_digits);
      state.expt += !state.after_point;
      if (significant) {
        digits[out->n_digits++] = str[i];
      } else if (str[i] != '0') {
        approximate = true;
      }
    } else if (str[i] == '.' && !state.after_point) {
      state.after_point = true;
    } else {
      break;
    }
  }

  out->expt = (int16_t)(out->n_digits ? state.expt : 0);
  out->kind = (sign < 0)
                ? (out->n_digits ? EXESS_NEGATIVE : EXESS_NEGATIVE_ZERO)
                : (out->n_digits ? EXESS_POSITIVE : EXESS_POSITIVE_ZERO);

  return RESULT(approximate ? EXESS_LOSS : EXESS_SUCCESS, i);
}
