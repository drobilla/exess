// Copyright 2019-2024 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#include "floating_decimal.h"
#include "read_utils.h"
#include "result.h"
#include "strtod.h"
#include "write_utils.h"

#include <exess/exess.h>

#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stddef.h>
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
decimal_metrics(const ExessFloatingDecimal count)
{
  const int expt =
    count.expt >= 0 ? (count.expt - (int)count.n_digits + 1) : count.expt;

  DecimalMetrics metrics = {EXESS_POINT_AFTER, 0U, 0U};

  if (count.expt >= (int)count.n_digits - 1) {
    metrics.point_loc      = EXESS_POINT_AFTER;
    metrics.n_zeros_before = (unsigned)count.expt - (count.n_digits - 1U);
    metrics.n_zeros_after  = 1U;
  } else if (count.expt < 0) {
    metrics.point_loc      = EXESS_POINT_BEFORE;
    metrics.n_zeros_before = 1U;
    metrics.n_zeros_after  = (unsigned)(-expt - 1);
  } else {
    metrics.point_loc = EXESS_POINT_BETWEEN;
  }

  return metrics;
}

static size_t
decimal_string_length(const ExessFloatingDecimal decimal)
{
  if (decimal.kind < EXESS_NEGATIVE_ZERO) {
    return 0U; // Invalid value
  }

  if (decimal.kind == EXESS_NEGATIVE_ZERO) {
    return 4U; // "-0.0"
  }

  if (decimal.kind == EXESS_POSITIVE_ZERO) {
    return 3U; // "0.0"
  }

  const DecimalMetrics metrics = decimal_metrics(decimal);
  const unsigned       n_zeros = metrics.n_zeros_before + metrics.n_zeros_after;
  const bool           is_negative = decimal.kind == EXESS_NEGATIVE;

  return is_negative + decimal.n_digits + 1 + n_zeros;
}

static size_t
copy_chars(char* const dest, const char* const src, const size_t n)
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

ExessResult
exess_read_decimal(double* const out, const char* const str)
{
  *out = (double)NAN;

  const size_t         i  = skip_whitespace(str);
  ExessFloatingDecimal in = {EXESS_NAN, 0U, 0, {0}};
  const ExessResult    r  = parse_decimal(&in, str + i);
  if (!r.status) {
    *out = decimal_to_double(in);
  }

  return result(r.status, i + r.count);
}

ExessResult
exess_write_decimal(const double value, const size_t buf_size, char* const buf)
{
  const ExessFloatingDecimal decimal = measure_double(value);
  if (!buf) {
    return result(EXESS_SUCCESS, decimal_string_length(decimal));
  }

  size_t i = 0;
  if (buf_size < 3) {
    return end_write(EXESS_NO_SPACE, buf_size, buf, 0);
  }

  if (decimal.kind < EXESS_NEGATIVE_ZERO) {
    return end_write(EXESS_BAD_VALUE, buf_size, buf, 0);
  }

  if (decimal.kind == EXESS_NEGATIVE_ZERO) {
    return write_special(4, "-0.0", buf_size, buf);
  }

  if (decimal.kind == EXESS_POSITIVE_ZERO) {
    return write_special(3, "0.0", buf_size, buf);
  }

  if (decimal.kind == EXESS_NEGATIVE) {
    buf[i++] = '-';
  }

  const DecimalMetrics metrics = decimal_metrics(decimal);
  const unsigned       n_zeros = metrics.n_zeros_before + metrics.n_zeros_after;
  if (buf_size - i <= decimal.n_digits + 1 + n_zeros) {
    return end_write(EXESS_NO_SPACE, buf_size, buf, 0);
  }

  if (metrics.point_loc == EXESS_POINT_AFTER) {
    i += copy_chars(buf + i, decimal.digits, decimal.n_digits);
    i += set_zeros(buf + i, metrics.n_zeros_before);
    i += copy_chars(buf + i, ".0", 2U);
  } else if (metrics.point_loc == EXESS_POINT_BEFORE) {
    i += copy_chars(buf + i, "0.", 2U);
    i += set_zeros(buf + i, metrics.n_zeros_after);
    i += copy_chars(buf + i, decimal.digits, decimal.n_digits);
  } else {
    assert(metrics.point_loc == EXESS_POINT_BETWEEN);
    assert(decimal.expt >= -1);

    const size_t n_before = (size_t)decimal.expt + 1U;
    i += copy_chars(buf + i, decimal.digits, n_before);

    buf[i++] = '.';

    const size_t n_after = decimal.n_digits - n_before;
    i += copy_chars(buf + i, decimal.digits + n_before, n_after);
  }

  return end_write(EXESS_SUCCESS, buf_size, buf, i);
}
