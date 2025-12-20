// Copyright 2019-2025 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#include "big_decimal.h"
#include "decimal_to_double.h"
#include "double_to_decimal.h"
#include "parse_decimal.h"
#include "read_utils.h"
#include "result.h"
#include "write_utils.h"

#include <exess/exess.h>

#include <assert.h>
#include <math.h>
#include <string.h>

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
decimal_metrics(const BigDecimal count)
{
  DecimalMetrics metrics = {EXESS_POINT_AFTER, 0U, 0U};

  if (count.expt >= (int)count.n_digits) {
    metrics.point_loc      = EXESS_POINT_AFTER;
    metrics.n_zeros_before = (unsigned)count.expt - count.n_digits;
    metrics.n_zeros_after  = 1U;
  } else if (count.expt < 0) {
    metrics.point_loc      = EXESS_POINT_BEFORE;
    metrics.n_zeros_before = 1U;
    metrics.n_zeros_after  = (unsigned)(-count.expt);
  } else {
    metrics.point_loc = EXESS_POINT_BETWEEN;
  }

  return metrics;
}

static size_t
write_zeros(const size_t n,
            const size_t buf_size,
            char* const  buf,
            const size_t i)
{
  (void)buf_size;
  assert(buf_size >= i + n);
  memset(buf + i, '0', n);
  return n;
}

ExessResult
exess_read_decimal(const char* const str, double* const out)
{
  *out = (double)NAN;

  const size_t      i  = skip_whitespace(str);
  BigDecimal        in = {EXESS_NAN, 0, 0U, {0}};
  const ExessResult r  = parse_decimal(str + i, &in);
  if (!r.status) {
    *out = decimal_to_double(in);
  }

  return RESULT(r.status, i + r.count);
}

ExessResult
exess_write_decimal(const double value, const size_t buf_size, char* const buf)
{
  const BigDecimal decimal = double_to_decimal(value, DBL_DECIMAL_DIG);

  if (decimal.kind < EXESS_NEGATIVE_ZERO) {
    return end_write(EXESS_BAD_VALUE, buf_size, buf, 0);
  }

  if (decimal.kind == EXESS_NEGATIVE_ZERO) {
    return write_special(4, "-0.0", buf_size, buf);
  }

  if (decimal.kind == EXESS_POSITIVE_ZERO) {
    return write_special(3, "0.0", buf_size, buf);
  }

  size_t i = 0;
  if (decimal.kind == EXESS_NEGATIVE) {
    i += write_char('-', buf_size, buf, i);
  }

  const DecimalMetrics metrics = decimal_metrics(decimal);
  const unsigned       n_zeros = metrics.n_zeros_before + metrics.n_zeros_after;
  const unsigned       n_total = decimal.n_digits + 1U + n_zeros;
  if (i + n_total >= buf_size) {
    return end_write(EXESS_NO_SPACE, buf_size, buf, i + n_total);
  }

  if (metrics.point_loc == EXESS_POINT_AFTER) {
    i += write_string(decimal.n_digits, decimal.digits, buf_size, buf, i);
    i += write_zeros(metrics.n_zeros_before, buf_size, buf, i);
    i += write_string(2, ".0", buf_size, buf, i);
  } else if (metrics.point_loc == EXESS_POINT_BEFORE) {
    i += write_string(2, "0.", buf_size, buf, i);
    i += write_zeros(metrics.n_zeros_after, buf_size, buf, i);
    i += write_string(decimal.n_digits, decimal.digits, buf_size, buf, i);
  } else {
    assert(metrics.point_loc == EXESS_POINT_BETWEEN);
    assert(decimal.expt >= 0);
    const size_t n_before = (size_t)decimal.expt;
    const size_t n_after  = decimal.n_digits - n_before;
    i += write_string(n_before, decimal.digits, buf_size, buf, i);
    i += write_char('.', buf_size, buf, i);
    i += write_string(n_after, decimal.digits + n_before, buf_size, buf, i);
  }

  return end_write(EXESS_SUCCESS, buf_size, buf, i);
}
