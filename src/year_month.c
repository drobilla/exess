// Copyright 2019-2026 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#include "date_utils.h"
#include "number_utils.h"
#include "read_utils.h"
#include "timezone.h"
#include "write_utils.h"
#include "year.h"

#include <exess/exess.h>

#include <string.h>

EXESS_NONBLOCKING ExessResult
exess_read_year_month(const char* const str, ExessYearMonth* const out)
{
  memset(out, 0, sizeof(*out));

  // Read YYYY-MM numbers
  size_t      i = skip_whitespace(str);
  ExessResult r = read_date_ym(&out->year, &out->month, str + i);

  // Read timezone if present
  r.count = i + r.count;
  return read_optional_timezone(&out->zone, str, r);
}

EXESS_NONBLOCKING ExessResult
exess_write_year_month(const ExessYearMonth value,
                       const size_t         buf_size,
                       char* const          buf)
{
  if (value.month < 1 || value.month > 12) {
    return end_write(EXESS_BAD_VALUE, buf_size, buf, 0);
  }

  // Write YYYY-MM numbers
  ExessResult r = write_year_number(value.year, buf_size, buf);
  r.count += write_char('-', buf_size, buf, r.count);
  r.count += write_two_digit_number(value.month, buf_size, buf, r.count);

  // Write timezone suffix and end write
  return write_timezone(value.zone, buf_size, buf, r);
}
