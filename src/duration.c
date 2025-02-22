// Copyright 2019-2025 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#include "read_utils.h"
#include "result.h"
#include "string_utils.h"
#include "time_utils.h"
#include "write_utils.h"

#include <exess/exess.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef enum { NONE, YEAR, MONTH, DAY, HOUR, MINUTE, SECOND } Field;

static ExessStatus
set_field(ExessDuration* const out,
          const Field          last_field,
          const Field          field,
          const uint64_t       value)
{
  if (value >= INT32_MAX) {
    return EXESS_OUT_OF_RANGE;
  }

  if (field <= last_field) {
    return EXESS_BAD_ORDER;
  }

  static const uint64_t factors[7] = {
    0UL, 12UL, 1UL, 24UL * 60UL * 60UL, 60UL * 60UL, 60UL, 1UL};

  const uint64_t factor = factors[field];
  int32_t* const target = (field < DAY) ? &out->months : &out->seconds;

  const int64_t new_value = (int64_t)*target + (int64_t)(factor * value);
  if (new_value >= INT32_MAX) {
    return EXESS_OUT_OF_RANGE;
  }

  *target = (int32_t)new_value;
  return EXESS_SUCCESS;
}

static ExessResult
read_duration_date(ExessDuration* const out, const char* const str)
{
  size_t      i          = 0U;
  ExessStatus st         = EXESS_SUCCESS;
  unsigned    last_field = 0U;

  while (!st && last_field < DAY && str[i] != 'T') {
    // Read the unsigned integer value
    uint64_t          value = 0U;
    const ExessResult r     = read_digits(&value, str + i);
    i += r.count;
    if (r.status) {
      st = r.count ? r.status : EXESS_SUCCESS;
      break;
    }

    // Read Y, M, or D field tag
    const Field field = (str[i] == 'Y')   ? YEAR
                        : (str[i] == 'M') ? MONTH
                        : (str[i] == 'D') ? DAY
                                          : NONE;
    if (!field) {
      return result(EXESS_EXPECTED_DATE_TAG, i);
    }

    // Set the field value and advance
    st         = set_field(out, (Field)last_field, field, value);
    last_field = field;
    ++i;
  }

  return i > 0 ? result(st, i) : result(EXESS_EXPECTED_DIGIT, i);
}

static ExessResult
read_duration_time(ExessDuration* const out, const char* const str)
{
  size_t      i          = 0U;
  ExessStatus st         = EXESS_SUCCESS;
  unsigned    last_field = 0U;

  while (!st && last_field < SECOND) {
    // Read the unsigned integer value
    uint64_t          value = 0U;
    const ExessResult r     = read_digits(&value, str + i);
    i += r.count;
    if (r.status) {
      st = r.count ? r.status : EXESS_SUCCESS;
      break;
    }

    Field field = YEAR;
    if (str[i] == '.') {
      uint32_t nanoseconds = 0U;
      if (!is_digit(str[++i])) {
        return result(EXESS_EXPECTED_DIGIT, i);
      }

      const ExessResult s = read_nanoseconds(&nanoseconds, str + i);
      i += s.count;
      if (str[i] != 'S') {
        return result(EXESS_EXPECTED_SECOND_TAG, i);
      }

      field            = SECOND;
      out->nanoseconds = (int32_t)nanoseconds;
    } else if (str[i] == 'H') {
      field = HOUR;
    } else if (str[i] == 'M') {
      field = MINUTE;
    } else if (str[i] == 'S') {
      field = SECOND;
    } else {
      return result(EXESS_EXPECTED_TIME_TAG, i);
    }

    // Set the field value and advance
    st         = set_field(out, (Field)last_field, field, value);
    last_field = (unsigned)field;
    ++i;
  }

  return i > 0 ? result(st, i) : result(EXESS_EXPECTED_DIGIT, i);
}

static int
compare_field(const int32_t lhs, const int32_t rhs)
{
  return lhs < rhs ? -1 : lhs == rhs ? 0 : 1;
}

int
exess_compare_duration(const ExessDuration lhs, const ExessDuration rhs)
{
  int cmp = compare_field(lhs.months, rhs.months);
  cmp     = cmp ? cmp : compare_field(lhs.seconds, rhs.seconds);
  cmp     = cmp ? cmp : compare_field(lhs.nanoseconds, rhs.nanoseconds);
  return cmp;
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
    ExessResult r = read_duration_date(out, str + i);
    if (r.status) {
      return result(r.status, i + r.count);
    }

    i += r.count;
  }

  if (str[i] == 'T') {
    ++i;

    ExessResult r = read_duration_time(out, str + i);
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

  return result(EXESS_SUCCESS, i);
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

static size_t
write_seconds(ExessResult*   r,
              const uint8_t  abs_seconds,
              const uint32_t abs_nanoseconds,
              const size_t   buf_size,
              char* const    buf,
              const size_t   i)
{
  *r = write_digits(abs_seconds, buf_size, buf, i);

  size_t len = r->count;
  if (!r->status && abs_nanoseconds) {
    len += write_nanoseconds(abs_nanoseconds, buf_size, buf, i + len);
  }

  return len + write_char('S', buf_size, buf, i + len);
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
  i += is_negative ? write_char('-', buf_size, buf, i) : 0U;
  i += write_char('P', buf_size, buf, i);

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
    if (abs_seconds + abs_nanoseconds) {
      i += write_seconds(&r, abs_seconds, abs_nanoseconds, buf_size, buf, i);
    }
  }

  return end_write(r.status, buf_size, buf, i);
}
