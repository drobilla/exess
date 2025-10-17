// Copyright 2019-2025 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#include "read_utils.h"
#include "time_utils.h"
#include "write_utils.h"

#include <exess/exess.h>

#include <string.h>

ExessOrder
exess_compare_time(const ExessTime lhs, const ExessTime rhs)
{
  const ExessDateTime lhs_datetime = {
    1972, 12U, 31U, lhs.zone, lhs.hour, lhs.minute, lhs.second, lhs.nanosecond};

  const ExessDateTime rhs_datetime = {
    1972, 12U, 31U, rhs.zone, rhs.hour, rhs.minute, rhs.second, rhs.nanosecond};

  return exess_compare_date_time(lhs_datetime, rhs_datetime);
}

ExessResult
exess_read_time(ExessTime* const out, const char* const str)
{
  memset(out, 0, sizeof(*out));

  size_t      i = skip_whitespace(str);
  ExessResult r = read_time(out, str + i);

  if (out->hour == 24) {
    if (out->minute || out->second || out->nanosecond) {
      r.status = EXESS_OUT_OF_RANGE;
    } else {
      out->hour = 0;
    }
  }

  r.count += i;
  return r;
}

ExessResult
exess_write_time(const ExessTime value, const size_t buf_size, char* const buf)
{
  if (value.hour == 24) {
    return end_write(EXESS_BAD_VALUE, buf_size, buf, 0);
  }

  const ExessResult r = write_time(value, buf_size, buf, 0);

  return end_write(r.status, buf_size, buf, r.count);
}
