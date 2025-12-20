// Copyright 2019-2021 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#include "big_decimal.h"
#include "char_utils.h"
#include "decimal_to_double.h"
#include "double_to_decimal.h"
#include "number_utils.h"
#include "parse_decimal.h"
#include "read_utils.h"
#include "result.h"
#include "scientific.h"
#include "write_utils.h"

#include <exess/exess.h>

#include <math.h>
#include <string.h>

static ExessResult
parse_double(const char* const str, BigDecimal* const out)
{
  typedef struct {
    BigDecimalKind kind;
    unsigned       length;
    const char*    string;
  } SpecialCase;

  static const SpecialCase special_cases[] = {
    {EXESS_NAN, 3U, "NaN"},
    {EXESS_NEGATIVE_INFINITY, 4U, "-INF"},
    {EXESS_POSITIVE_INFINITY, 3U, "INF"},
    {EXESS_POSITIVE_INFINITY, 4U, "+INF"},
    {EXESS_NAN, 0U, ""},
  };

  // Handle non-numeric special cases
  for (const SpecialCase* c = special_cases; c->length; ++c) {
    if (!strcmp(str, c->string)) {
      out->kind = c->kind;
      return RESULT(EXESS_SUCCESS, c->length);
    }
  }

  // Read mantissa as a decimal
  const ExessResult r = parse_decimal(str, out);
  if (r.status) {
    return r;
  }

  size_t i = r.count;

  // Read exponent
  int abs_expt  = 0;
  int expt_sign = 1;
  if (str[i] == 'e' || str[i] == 'E') {
    ++i;

    if (!is_sign(str[i]) && !is_digit(str[i])) {
      return RESULT(EXESS_EXPECTED_DIGIT, i);
    }

    i += read_sign(&expt_sign, &str[i]);
    while (is_digit(str[i])) {
      abs_expt = (abs_expt * 10) + (str[i++] - '0');
    }
  }

  // Calculate final output exponent
  out->expt += expt_sign * abs_expt;

  return RESULT(r.status, i);
}

ExessResult
exess_read_double(const char* const str, double* const out)
{
  const size_t      i  = skip_whitespace(str);
  BigDecimal        in = {EXESS_NAN, 0, 0U, {0}};
  const ExessResult r  = parse_double(str + i, &in);

  *out = r.status ? (double)NAN : decimal_to_double(in);
  return RESULT(r.status, i + r.count);
}

ExessResult
exess_write_double(const double value, const size_t buf_size, char* const buf)
{
  const BigDecimal  decimal = double_to_decimal(value, DBL_DECIMAL_DIG);
  const ExessResult r       = write_scientific(decimal, buf_size, buf);
  return end_write(r.status, buf_size, buf, r.count);
}
