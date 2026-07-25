// Copyright 2019-2026 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#include "date_utils.h"
#include "number_utils.h"
#include "read_utils.h"
#include "result.h"
#include "timezone.h"
#include "write_utils.h"

#include <exess/exess.h>

#include <string.h>

EXESS_NONBLOCKING ExessResult
exess_read_month(const char* const str, ExessMonth* const out)
{
  memset(out, 0, sizeof(*out));

  // Skip leading absent-year delimiter
  size_t i = skip_whitespace(str);
  if (str[i] != '-') {
    return RESULT(EXESS_EXPECTED_DASH, i);
  }

  // Read (leading month delimiter and) month
  ++i;
  ExessResult r = read_date_m(&out->month, str + i);

  // Read timezone if present
  r.count = i + r.count;
  return read_optional_timezone(&out->zone, str, r);
}

EXESS_NONBLOCKING ExessResult
exess_write_month(const ExessMonth value,
                  const size_t     buf_size,
                  char* const      buf)
{
  if (value.month < 1 || value.month > 12) {
    return end_write(EXESS_BAD_VALUE, buf_size, buf, 0);
  }

  // Write -MM number
  ExessResult r = {EXESS_SUCCESS, 0};
  r.count += write_char('-', buf_size, buf, r.count);
  r.count += write_char('-', buf_size, buf, r.count);
  r.count += write_two_digit_number(value.month, buf_size, buf, r.count);

  return write_timezone(value.zone, buf_size, buf, r);
}
