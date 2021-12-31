// Copyright 2019-2021 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#include "decimal.h"
#include "digits.h"
#include "read_utils.h"
#include "result.h"
#include "string_utils.h"
#include "strtod.h"
#include "warnings.h"
#include "write_utils.h"

#include "exess/exess.h"

#include <math.h>
#include <stdbool.h>
#include <string.h>

#include <assert.h>

typedef enum {
  EXESS_POINT_AFTER,   ///< Decimal point is after all significant digits
  EXESS_POINT_BEFORE,  ///< Decimal point is before all significant digits
  EXESS_POINT_BETWEEN, ///< Decimal point is between significant digits
} ExessPointLocation;

typedef struct {
  ExessPointLocation point_loc;      ///< Location of decimal point
  unsigned           n_zeros_before; ///< Number of extra zeros before point
  unsigned           n_zeros_after;  ///< Number of extra zeros after point
} DecimalMetrics;

static DecimalMetrics
decimal_metrics(const ExessDecimalDouble count)
{
  const int expt =
    count.expt >= 0 ? (count.expt - (int)count.n_digits + 1) : count.expt;

  DecimalMetrics metrics = {EXESS_POINT_AFTER, 0u, 0u};

  if (count.expt >= (int)count.n_digits - 1) {
    metrics.point_loc      = EXESS_POINT_AFTER;
    metrics.n_zeros_before = (unsigned)count.expt - (count.n_digits - 1u);
    metrics.n_zeros_after  = 1u;
  } else if (count.expt < 0) {
    metrics.point_loc      = EXESS_POINT_BEFORE;
    metrics.n_zeros_before = 1u;
    metrics.n_zeros_after  = (unsigned)(-expt - 1);
  } else {
    metrics.point_loc = EXESS_POINT_BETWEEN;
  }

  return metrics;
}

static ExessNumberKind
number_kind(const double d)
{
  EXESS_DISABLE_CONVERSION_WARNINGS
  const int  fpclass     = fpclassify(d);
  const bool is_negative = signbit(d);
  EXESS_RESTORE_WARNINGS

  switch (fpclass) {
  case FP_ZERO:
    return is_negative ? EXESS_NEGATIVE_ZERO : EXESS_POSITIVE_ZERO;
  case FP_INFINITE:
    return is_negative ? EXESS_NEGATIVE_INFINITY : EXESS_POSITIVE_INFINITY;
  case FP_NORMAL:
  case FP_SUBNORMAL:
    return is_negative ? EXESS_NEGATIVE : EXESS_POSITIVE;
  default:
    break;
  }

  return EXESS_NAN;
}

ExessDecimalDouble
exess_measure_decimal(const double d, const unsigned max_precision)
{
  ExessDecimalDouble value = {number_kind(d), 0, 0, {0}};

  if (value.kind != EXESS_NEGATIVE && value.kind != EXESS_POSITIVE) {
    return value;
  }

  // Get decimal digits
  const double          abs_d = fabs(d);
  const ExessDigitCount count =
    exess_digits(abs_d, value.digits, max_precision);

  assert(count.count == 1 || value.digits[count.count - 1] != '0');

  value.n_digits = count.count;
  value.expt     = count.expt;

  return value;
}

ExessDecimalDouble
exess_measure_float(const float f)
{
  return exess_measure_decimal((double)f, FLT_DECIMAL_DIG);
}

ExessDecimalDouble
exess_measure_double(const double d)
{
  return exess_measure_decimal(d, DBL_DECIMAL_DIG);
}

static size_t
exess_decimal_double_string_length(const ExessDecimalDouble decimal)
{
  switch (decimal.kind) {
  case EXESS_NEGATIVE:
    break;
  case EXESS_NEGATIVE_INFINITY:
    return 0;
  case EXESS_NEGATIVE_ZERO:
    return 4;
  case EXESS_POSITIVE_ZERO:
    return 3;
  case EXESS_POSITIVE:
    break;
  case EXESS_POSITIVE_INFINITY:
  case EXESS_NAN:
    return 0;
  }

  const DecimalMetrics metrics = decimal_metrics(decimal);
  const unsigned       n_zeros = metrics.n_zeros_before + metrics.n_zeros_after;
  const bool           is_negative = decimal.kind == EXESS_NEGATIVE;

  return is_negative + decimal.n_digits + 1 + n_zeros;
}

static size_t
copy_digits(char* const dest, const char* const src, const size_t n)
{
  memcpy(dest, src, n);
  return n;
}

static size_t
set_zeros(char* const dest, const size_t n)
{
  memset(dest, '0', n);
  return n;
}

static ExessResult
read_decimal_number(double* const out, const char* const str)
{
  *out = (double)NAN;

  if (str[0] == '+' || str[0] == '-') {
    if (str[1] != '.' && !is_digit(str[1])) {
      return result(EXESS_EXPECTED_DIGIT, 1);
    }
  } else if (str[0] != '.' && !is_digit(str[0])) {
    return result(EXESS_EXPECTED_DIGIT, 0);
  }

  const size_t       i  = skip_whitespace(str);
  ExessDecimalDouble in = {EXESS_NAN, 0u, 0, {0}};
  const ExessResult  r  = parse_decimal(&in, str + i);

  if (!r.status) {
    *out = parsed_double_to_double(in);
  }

  return result(r.status, i + r.count);
}

ExessResult
exess_read_decimal(double* const out, const char* const str)
{
  const size_t      i = skip_whitespace(str);
  const ExessResult r = read_decimal_number(out, str + i);

  return end_read(r.status, str, i + r.count);
}

ExessResult
exess_write_decimal_double(const ExessDecimalDouble decimal,
                           const size_t             buf_size,
                           char* const              buf)
{
  if (!buf) {
    return result(EXESS_SUCCESS, exess_decimal_double_string_length(decimal));
  }

  size_t i = 0;
  if (buf_size < 3) {
    return end_write(EXESS_NO_SPACE, buf_size, buf, 0);
  }

  switch (decimal.kind) {
  case EXESS_NEGATIVE:
    buf[i++] = '-';
    break;
  case EXESS_NEGATIVE_INFINITY:
    return end_write(EXESS_BAD_VALUE, buf_size, buf, 0);
  case EXESS_NEGATIVE_ZERO:
    return write_special(4, "-0.0", buf_size, buf);
  case EXESS_POSITIVE_ZERO:
    return write_special(3, "0.0", buf_size, buf);
  case EXESS_POSITIVE:
    break;
  case EXESS_POSITIVE_INFINITY:
  case EXESS_NAN:
    return end_write(EXESS_BAD_VALUE, buf_size, buf, 0);
  }

  const DecimalMetrics metrics = decimal_metrics(decimal);
  const unsigned       n_zeros = metrics.n_zeros_before + metrics.n_zeros_after;
  if (buf_size - i <= decimal.n_digits + 1 + n_zeros) {
    return end_write(EXESS_NO_SPACE, buf_size, buf, 0);
  }

  if (metrics.point_loc == EXESS_POINT_AFTER) {
    i += copy_digits(buf + i, decimal.digits, decimal.n_digits);
    i += set_zeros(buf + i, metrics.n_zeros_before);
    buf[i++] = '.';
    buf[i++] = '0';
  } else if (metrics.point_loc == EXESS_POINT_BEFORE) {
    buf[i++] = '0';
    buf[i++] = '.';
    i += set_zeros(buf + i, metrics.n_zeros_after);
    i += copy_digits(buf + i, decimal.digits, decimal.n_digits);
  } else {
    assert(metrics.point_loc == EXESS_POINT_BETWEEN);
    assert(decimal.expt >= -1);

    const size_t n_before = (size_t)decimal.expt + 1u;
    const size_t n_after  = decimal.n_digits - n_before;

    i += copy_digits(buf + i, decimal.digits, n_before);
    buf[i++] = '.';
    memcpy(buf + i, decimal.digits + n_before, n_after);
    i += n_after;
  }

  return end_write(EXESS_SUCCESS, buf_size, buf, i);
}

ExessResult
exess_write_decimal(const double value, const size_t n, char* const buf)
{
  const ExessDecimalDouble decimal = exess_measure_double(value);

  return exess_write_decimal_double(decimal, n, buf);
}
