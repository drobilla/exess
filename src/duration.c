/*
  Copyright 2019-2021 David Robillard <d@drobilla.net>

  Permission to use, copy, modify, and/or distribute this software for any
  purpose with or without fee is hereby granted, provided that the above
  copyright notice and this permission notice appear in all copies.

  THIS SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
  WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
  MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
  ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
  WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
  ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
  OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/

#include "read_utils.h"
#include "string_utils.h"
#include "time_utils.h"
#include "write_utils.h"

#include "exess/exess.h"

#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef enum { YEAR, MONTH, DAY, HOUR, MINUTE, SECOND } Field;

static ExessStatus
set_field(ExessDuration* const out,
          const Field          current_field,
          const Field          field,
          const uint32_t       value)
{
  if (value >= INT32_MAX) {
    return EXESS_OUT_OF_RANGE;
  }

  if (field < current_field) {
    return EXESS_BAD_ORDER;
  }

  switch (field) {
  case YEAR:
    out->months = (int32_t)(12 * lrint(value));
    break;
  case MONTH:
    out->months = (int32_t)(out->months + lrint(value));
    break;
  case DAY:
    out->seconds = (int32_t)(24 * 60 * 60 * lrint(value));
    break;
  case HOUR:
    out->seconds = (int32_t)(out->seconds + 60 * 60 * lrint(value));
    break;
  case MINUTE:
    out->seconds = (int32_t)(out->seconds + 60 * lrint(value));
    break;
  case SECOND:
    out->seconds = (int32_t)(out->seconds + lrint(value));
    break;
  }

  return EXESS_SUCCESS;
}

static ExessResult
read_date(ExessDuration* const out, const Field field, const char* const str)
{
  uint32_t    value = 0;
  ExessResult r     = exess_read_uint(&value, str);
  if (r.status > EXESS_EXPECTED_END) {
    return r;
  }

  size_t i = r.count;
  switch (str[i]) {
  case 'Y':
    if ((r.status = set_field(out, field, YEAR, value))) {
      return r;
    }

    ++i;
    if (str[i] != 'T' && !is_end(str[i])) {
      r = read_date(out, MONTH, str + i);
      i += r.count;
    }
    break;

  case 'M':
    if ((r.status = set_field(out, field, MONTH, value))) {
      return r;
    }

    ++i;
    if (str[i] != 'T' && !is_end(str[i])) {
      r = read_date(out, DAY, str + i);
      i += r.count;
    }
    break;

  case 'D':
    if ((r.status = set_field(out, field, DAY, value))) {
      return r;
    }

    ++i;
    break;

  default:
    return result(EXESS_EXPECTED_DATE_TAG, i);
  }

  return result(r.status, i);
}

static ExessResult
read_time(ExessDuration* const out, const Field field, const char* const str)
{
  uint32_t    value = 0;
  ExessResult r     = exess_read_uint(&value, str);
  if (r.status > EXESS_EXPECTED_END) {
    return r;
  }

  size_t      i    = r.count;
  ExessResult next = {EXESS_SUCCESS, 0};
  switch (str[i]) {
  case '.': {
    if (!is_digit(str[++i])) {
      return result(EXESS_EXPECTED_DIGIT, i);
    }

    uint32_t nanoseconds = 0;

    r = read_nanoseconds(&nanoseconds, str + i);
    i += r.count;

    if (str[i] != 'S') {
      return result(EXESS_EXPECTED_TIME_TAG, i);
    }

    r.status         = set_field(out, field, SECOND, value);
    out->nanoseconds = (int32_t)nanoseconds;

    break;
  }

  case 'H':
    r.status = set_field(out, field, HOUR, value);
    if (!is_end(str[i + 1])) {
      next = read_time(out, MINUTE, str + i + 1);
    }
    break;

  case 'M':
    r.status = set_field(out, field, MINUTE, value);
    if (!is_end(str[i + 1])) {
      next = read_time(out, SECOND, str + i + 1);
    }
    break;

  case 'S':
    r.status = set_field(out, field, SECOND, value);
    break;

  default:
    return result(EXESS_EXPECTED_TIME_TAG, i);
  }

  if (r.status) {
    return r;
  }

  return result(next.status, i + 1 + next.count);
}

int
exess_duration_compare(const ExessDuration lhs, const ExessDuration rhs)
{
  return lhs.months < rhs.months             ? -1
         : lhs.months > rhs.months           ? 1
         : lhs.seconds < rhs.seconds         ? -1
         : lhs.seconds > rhs.seconds         ? 1
         : lhs.nanoseconds < rhs.nanoseconds ? -1
         : lhs.nanoseconds > rhs.nanoseconds ? 1
                                             : 0;
}

ExessResult
exess_read_duration(ExessDuration* const out, const char* const str)
{
  memset(out, 0, sizeof(*out));

  size_t i           = skip_whitespace(str);
  bool   is_negative = false;
  if (str[i] == '-') {
    is_negative = true;
    ++i;
  }

  if (str[i] != 'P') {
    return result(EXESS_EXPECTED_DURATION, i);
  }

  ++i;

  if (str[i] != 'T') {
    ExessResult r = read_date(out, YEAR, str + i);
    if (r.status) {
      return result(r.status, i + r.count);
    }

    i += r.count;

    if (!is_end(str[i]) && str[i] != 'T') {
      return result(EXESS_EXPECTED_TIME_SEP, i);
    }
  }

  if (str[i] == 'T') {
    ++i;

    ExessResult r = read_time(out, HOUR, str + i);
    if (r.status) {
      return result(r.status, i + r.count);
    }

    i += r.count;
  }

  if (is_negative) {
    out->months      = -out->months;
    out->seconds     = -out->seconds;
    out->nanoseconds = -out->nanoseconds;
  }

  return end_read(EXESS_SUCCESS, str, i);
}

static size_t
write_int_field(ExessResult*   r,
                const uint32_t value,
                const char     tag,
                const size_t   buf_size,
                char* const    buf,
                const size_t   i)
{
  if (!r->status) {
    if (value == 0) {
      *r = result(EXESS_SUCCESS, 0);
    } else if (!buf) {
      *r = exess_write_uint(value, buf_size, buf);
      ++r->count;
    } else {
      *r = exess_write_uint(value, buf_size - i, buf + i);
      if (!r->status) {
        buf[i + r->count++] = tag;
      }
    }
  }

  return r->count;
}

ExessResult
exess_write_duration(const ExessDuration value,
                     const size_t        buf_size,
                     char* const         buf)
{
  // Write zero as a special case
  size_t i = 0;
  if (value.months == 0 && value.seconds == 0 && value.nanoseconds == 0) {
    i += write_string(3, "P0Y", buf_size, buf, i);
    return end_write(EXESS_SUCCESS, buf_size, buf, i);
  }

  if (value.months == INT32_MIN || value.seconds == INT32_MIN) {
    return end_write(EXESS_OUT_OF_RANGE, buf_size, buf, 0);
  }

  const bool is_negative =
    (value.months < 0 || value.seconds < 0 || value.nanoseconds < 0);

  if (is_negative &&
      (value.months > 0 || value.seconds > 0 || value.nanoseconds > 0)) {
    return end_write(EXESS_BAD_VALUE, buf_size, buf, 0);
  }

  // Write duration prefix
  if (value.months < 0 || value.seconds < 0 || value.nanoseconds < 0) {
    i += write_string(2, "-P", buf_size, buf, i);
  } else {
    i += write_char('P', buf_size, buf, i);
  }

  const uint32_t abs_years   = (uint32_t)(abs(value.months) / 12);
  const uint32_t abs_months  = (uint32_t)(abs(value.months) % 12);
  const uint32_t abs_days    = (uint32_t)(abs(value.seconds) / (24 * 60 * 60));
  const uint32_t abs_hours   = (uint32_t)(abs(value.seconds) / 60 / 60 % 24);
  const uint32_t abs_minutes = (uint32_t)(abs(value.seconds) / 60 % 60);
  const uint8_t  abs_seconds = (uint8_t)(abs(value.seconds) % 60);
  const uint32_t abs_nanoseconds = (uint32_t)abs(value.nanoseconds);

  // Write date segments if present
  ExessResult r = result(EXESS_SUCCESS, 0);
  i += write_int_field(&r, abs_years, 'Y', buf_size, buf, i);
  i += write_int_field(&r, abs_months, 'M', buf_size, buf, i);
  i += write_int_field(&r, abs_days, 'D', buf_size, buf, i);

  // Write time segments if present
  const bool has_time = abs_hours + abs_minutes + abs_seconds + abs_nanoseconds;
  if (has_time && !r.status) {
    i += write_char('T', buf_size, buf, i);
    i += write_int_field(&r, abs_hours, 'H', buf_size, buf, i);
    i += write_int_field(&r, abs_minutes, 'M', buf_size, buf, i);

    if (abs_seconds != 0 || abs_nanoseconds != 0) {
      r = write_digits(abs_seconds, buf_size, buf, i);
      i += r.count;

      if (!r.status && abs_nanoseconds > 0) {
        i += write_nanoseconds(abs_nanoseconds, buf_size, buf, i);
      }

      i += write_char('S', buf_size, buf, i);
    }
  }

  return end_write(r.status, buf_size, buf, i);
}
