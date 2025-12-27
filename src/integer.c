// Copyright 2019-2025 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#include "big_decimal.h"
#include "char_utils.h"
#include "number_utils.h"
#include "read_utils.h"
#include "result.h"
#include "write_utils.h"

#include <exess/exess.h>

#include <assert.h>
#include <stddef.h>
#include <stdint.h>

// Value has a BigDecimal header, char digits, and a null byte
static const size_t min_value_size = sizeof(BigDecimal) + 1U;

ExessVariableResult
exess_read_integer(const char* const str,
                   const size_t      out_size,
                   void* const       out)
{
  if (out_size < min_value_size) {
    return VRESULT(EXESS_NO_SPACE, 0U, min_value_size);
  }

  BigDecimal* const d           = (BigDecimal*)out;
  char* const       digits      = (char*)out + sizeof(*d);
  const size_t      digits_size = out_size - sizeof(*d) - 1U;

  d->kind     = EXESS_NAN;
  d->n_digits = 0U;
  d->expt     = 0;

  // Skip leading whitespace and read sign if present
  size_t i    = skip_whitespace(str);
  int    sign = 1;
  i += read_sign(&sign, &str[i]);

  // Ensure the first character is a digit
  if (!is_digit(str[i])) {
    return VRESULT(EXESS_EXPECTED_DIGIT, i, 0U);
  }

  // Skip leading zeros
  while (str[i] == '0') {
    ++i;
  }

  // Read digits
  unsigned n_trailing_zeros = 0U;
  for (; is_digit(str[i]); ++i) {
    if (d->n_digits == digits_size) {
      return VRESULT(EXESS_OUT_OF_RANGE, i, 0U);
    }

    ++d->expt;
    digits[d->n_digits++] = str[i];
    n_trailing_zeros      = (str[i] == '0') ? (n_trailing_zeros + 1U) : 0U;
  }

  // Trim trailing zeros
  assert(n_trailing_zeros <= d->n_digits);
  d->n_digits = (uint8_t)(d->n_digits - n_trailing_zeros);

  digits[d->n_digits] = '\0';

  d->kind = !d->n_digits ? EXESS_POSITIVE_ZERO
            : (sign > 0) ? EXESS_POSITIVE
                         : EXESS_NEGATIVE;

  return VRESULT(EXESS_SUCCESS, i, sizeof(*d) + d->n_digits + 1U);
}

ExessResult
exess_write_integer(const size_t      value_size,
                    const void* const value,
                    const size_t      buf_size,
                    char* const       buf)
{
  const BigDecimal  decimal = *(const BigDecimal*)value;
  const char* const digits  = (const char*)value + sizeof(decimal);

  if (decimal.kind < EXESS_NEGATIVE_ZERO ||
      value_size < sizeof(decimal) + decimal.n_digits ||
      decimal.expt < (int)decimal.n_digits) {
    return end_write(EXESS_BAD_VALUE, buf_size, buf, 0);
  }

  size_t i = 0U;
  if (decimal.kind == EXESS_NEGATIVE) {
    i += write_char('-', buf_size, buf, i);
  }

  const unsigned n_extra_zeros =
    ((unsigned)decimal.expt - decimal.n_digits) + !decimal.n_digits;

  const size_t n_total = i + decimal.n_digits + n_extra_zeros;
  if (n_total >= buf_size) {
    return end_write(EXESS_NO_SPACE, buf_size, buf, n_total);
  }

  i += write_string(decimal.n_digits, digits, buf_size, buf, i);
  i += write_zeros(n_extra_zeros, buf_size, buf, i);

  return end_write(EXESS_SUCCESS, buf_size, buf, i);
}
