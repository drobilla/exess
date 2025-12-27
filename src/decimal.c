// Copyright 2019-2025 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#include "big_decimal.h"
#include "number_utils.h"
#include "parse_decimal.h"
#include "read_utils.h"
#include "result.h"
#include "write_utils.h"

#include <exess/exess.h>

#include <assert.h>
#include <stddef.h>

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

ExessVariableResult
exess_read_decimal(const char* const str,
                   const size_t      out_size,
                   void* const       out)
{
  if (out_size < sizeof(BigDecimal) + 2U) {
    return VRESULT(EXESS_NO_SPACE, 0U, sizeof(BigDecimal) + 2U);
  }

  char* const  digits      = (char*)out + sizeof(BigDecimal);
  const size_t digits_size = out_size - sizeof(BigDecimal) - 1U;

  const size_t      i = skip_whitespace(str);
  BigDecimal        d = {EXESS_NAN, 0U, 0};
  const ExessResult r = parse_decimal(str + i, &d, digits_size, digits);

  // Trim trailing zeros
  if (r.status <= EXESS_LOSS) {
    while (d.n_digits > 1 && digits[d.n_digits - 1] == '0') {
      --d.n_digits;
    }
  }

  *(BigDecimal*)out  = d;    // Set output header
  digits[d.n_digits] = '\0'; // Terminate digits for safety/convenience

  return VRESULT(r.status, i + r.count, sizeof(d) + d.n_digits + 1U);
}

ExessResult
exess_write_decimal(const size_t      value_size,
                    const void* const value,
                    const size_t      buf_size,
                    char* const       buf)
{
  const BigDecimal  decimal = *(const BigDecimal*)value;
  const char* const digits  = (const char*)value + sizeof(decimal);

  if (decimal.kind < EXESS_NEGATIVE_ZERO ||
      value_size < sizeof(decimal) + decimal.n_digits) {
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
    i += write_string(decimal.n_digits, digits, buf_size, buf, i);
    i += write_zeros(metrics.n_zeros_before, buf_size, buf, i);
    i += write_string(2, ".0", buf_size, buf, i);
  } else if (metrics.point_loc == EXESS_POINT_BEFORE) {
    i += write_string(2, "0.", buf_size, buf, i);
    i += write_zeros(metrics.n_zeros_after, buf_size, buf, i);
    i += write_string(decimal.n_digits, digits, buf_size, buf, i);
  } else {
    assert(metrics.point_loc == EXESS_POINT_BETWEEN);
    assert(decimal.expt >= 0);
    const size_t n_before = (size_t)decimal.expt;
    const size_t n_after  = decimal.n_digits - n_before;
    i += write_string(n_before, digits, buf_size, buf, i);
    i += write_char('.', buf_size, buf, i);
    i += write_string(n_after, digits + n_before, buf_size, buf, i);
  }

  return end_write(EXESS_SUCCESS, buf_size, buf, i);
}
