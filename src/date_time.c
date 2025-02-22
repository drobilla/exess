// Copyright 2019-2023 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#include "date_utils.h"
#include "result.h"
#include "string_utils.h"
#include "time_utils.h"
#include "write_utils.h"

#include <exess/exess.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

static inline ExessDateTime
infinite_future(const bool is_utc)
{
  const ExessDateTime r = {INT16_MAX,
                           UINT8_MAX,
                           UINT8_MAX,
                           is_utc,
                           UINT8_MAX,
                           UINT8_MAX,
                           UINT8_MAX,
                           UINT32_MAX};

  return r;
}

static inline ExessDateTime
infinite_past(const bool is_utc)
{
  const ExessDateTime r = {INT16_MIN, 0, 0, is_utc, 0, 0, 0, 0};

  return r;
}

static int32_t
modulo(const int32_t a, const int32_t low, const int32_t high)
{
  return ((a - low) % (high - low)) + low;
}

static int32_t
quotient(const int32_t a, const int32_t low, const int32_t high)
{
  return (a - low) / (high - low);
}

static int
compare_field(const unsigned lhs, const unsigned rhs)
{
  return lhs < rhs ? -1 : lhs == rhs ? 0 : 1;
}

static int
compare_date_time_determinate(const ExessDateTime lhs, const ExessDateTime rhs)
{
  if (lhs.year != rhs.year) {
    return lhs.year < rhs.year ? -1 : 1;
  }

  int cmp = compare_field(lhs.month, rhs.month);
  cmp     = cmp ? cmp : compare_field(lhs.day, rhs.day);
  cmp     = cmp ? cmp : compare_field(lhs.hour, rhs.hour);
  cmp     = cmp ? cmp : compare_field(lhs.minute, rhs.minute);
  cmp     = cmp ? cmp : compare_field(lhs.second, rhs.second);
  cmp     = cmp ? cmp : compare_field(lhs.nanosecond, rhs.nanosecond);
  return cmp;
}

static ExessDateTime
to_utc(const ExessDateTime s, const ExessDuration offset)
{
  ExessDateTime r = exess_add_date_time_duration(s, offset);
  r.is_utc        = true;
  return r;
}

static int
compare_date_time_partial(const ExessDateTime lhs, const ExessDateTime rhs)
{
  // See https://www.w3.org/TR/xmlschema-2/#dateTime-order
  // and https://www.w3.org/TR/xmlschema11-2/#theSevenPropertyModel

  static const ExessDuration plus_14h  = {0U, 14 * 60 * 60, 0};
  static const ExessDuration minus_14h = {0U, -14 * 60 * 60, 0};

  if (lhs.is_utc) {
    const ExessDateTime r_minus = to_utc(rhs, minus_14h);
    if (compare_date_time_determinate(lhs, r_minus) < 0) {
      return -1;
    }

    const ExessDateTime r_plus = to_utc(rhs, plus_14h);
    if (compare_date_time_determinate(lhs, r_plus) > 0) {
      return 1;
    }

    // Indeterminate, arbitrarily put local time first
    return 1;
  }

  const ExessDateTime l_plus = to_utc(lhs, plus_14h);
  if (compare_date_time_determinate(l_plus, rhs) < 0) {
    return -1;
  }

  const ExessDateTime l_minus = to_utc(lhs, minus_14h);
  if (compare_date_time_determinate(l_minus, rhs) > 0) {
    return 1;
  }

  // Indeterminate, arbitrarily put local time first
  return -1;
}

int
exess_compare_date_time(const ExessDateTime lhs, const ExessDateTime rhs)
{
  return (lhs.is_utc == rhs.is_utc) ? compare_date_time_determinate(lhs, rhs)
                                    : compare_date_time_partial(lhs, rhs);
}

static int32_t
add_field(const int32_t  lhs,
          const int32_t  rhs,
          const int32_t  max,
          int32_t* const carry)
{
  const int32_t temp = lhs + rhs + *carry;

  if (temp < 0) {
    *carry = temp / max - 1;
    return max + (temp % max);
  }

  *carry = temp / max;
  return temp % max;
}

/**
   Set the day, carrying into into months and years as necessary.

   Note that the algorithm in the spec first clamps here, but we don't because
   no such dateTime should exist (exess_read_date_time refuses to read them).
   This might return the infinite past or future.
*/
EXESS_CONST_FUNC static ExessDateTime
carry_set_day(ExessDateTime e, int day)
{
  while (day < 1 || day > days_in_month(e.year, e.month)) {
    if (day < 1) {
      if (--e.month == 0) {
        if (e.year == INT16_MIN) {
          return infinite_past(e.is_utc);
        }

        --e.year;
        e.month = 12;
      }
      day += days_in_month(e.year, e.month);
    } else {
      day -= days_in_month(e.year, e.month);
      if (++e.month > 12) {
        if (e.year == INT16_MAX) {
          return infinite_future(e.is_utc);
        }

        ++e.year;
        e.month = (uint8_t)modulo(e.month, 1, 13);
      }
    }
  }

  e.day = (uint8_t)day;
  return e;
}

ExessDateTime
exess_add_date_time_duration(const ExessDateTime s, const ExessDuration d)
{
  // See https://www.w3.org/TR/xmlschema-2/#adding-durations-to-dateTimes
  // and https://www.w3.org/TR/xmlschema11-2/#sec-dt-arith
  // This algorithm is modified here to support subtraction when d is negative

  static const int32_t giga = 1000000000;

  const int32_t d_year   = d.months / 12;
  const int32_t d_month  = d.months % 12;
  const int32_t d_day    = d.seconds / (24 * 60 * 60);
  const int32_t d_hour   = d.seconds / 60 / 60 % 24;
  const int32_t d_minute = d.seconds / 60 % 60;
  const int32_t d_second = d.seconds % 60;

  ExessDateTime e     = {0, 0U, 0U, s.is_utc, 0U, 0U, 0U, 0U};
  int32_t       temp  = 0;
  int32_t       carry = 0;

  // Months (may be modified additionally below)
  temp = s.month + d_month;
  if (temp <= 0) {
    e.month = (uint8_t)(12 + modulo(temp, 1, 13));
    carry   = quotient(temp, 1, 13) - 1;
  } else {
    e.month = (uint8_t)modulo(temp, 1, 13);
    carry   = quotient(temp, 1, 13);
  }

  // Years (may be modified additionally below)
  temp  = s.year + d_year + carry;
  carry = 0;
  if (temp > INT16_MAX) {
    return infinite_future(s.is_utc);
  }

  if (temp < INT16_MIN) {
    return infinite_past(s.is_utc);
  }

  e.year = (int16_t)temp;

  // Day time

  e.nanosecond =
    (uint32_t)add_field((int32_t)s.nanosecond, d.nanoseconds, giga, &carry);

  e.second = (uint8_t)add_field(s.second, d_second, 60, &carry);
  e.minute = (uint8_t)add_field(s.minute, d_minute, 60, &carry);
  e.hour   = (uint8_t)add_field(s.hour, d_hour, 24, &carry);

  return carry_set_day(e, s.day + d_day + carry);
}

ExessResult
exess_read_date_time(ExessDateTime* const out, const char* const str)
{
  out->year  = 0;
  out->month = 0;
  out->day   = 0;

  // Read date
  ExessDate         date = {0, 0U, 0U, EXESS_LOCAL};
  const ExessResult dr   = read_date_numbers(&date, str);
  if (dr.status) {
    return dr;
  }

  size_t i = dr.count;
  if (str[i] != 'T') {
    return result(EXESS_EXPECTED_TIME_SEP, i);
  }

  ++i;

  // Read time
  ExessTime         time = {EXESS_LOCAL, 0U, 0U, 0U, 0U};
  const ExessResult tr   = exess_read_time(&time, str + i);
  if (tr.status) {
    return result(tr.status, i + tr.count);
  }

  i += tr.count;

  const ExessDateTime datetime = {date.year,
                                  date.month,
                                  date.day,
                                  time.zone != EXESS_LOCAL,
                                  time.hour,
                                  time.minute,
                                  time.second,
                                  time.nanosecond};

  if (datetime.is_utc) {
    const ExessDuration tz_duration = {0U, -time.zone * 15 * 60, 0};

    *out = exess_add_date_time_duration(datetime, tz_duration);
  } else {
    *out = datetime;
  }

  return result(EXESS_SUCCESS, i);
}

ExessResult
exess_write_date_time(const ExessDateTime value,
                      const size_t        buf_size,
                      char* const         buf)
{
  const ExessTimezone local = {EXESS_LOCAL};
  const ExessDate     date  = {value.year, value.month, value.day, local};
  const ExessTimezone zone  = {value.is_utc ? EXESS_UTC : EXESS_LOCAL};

  const ExessTime time = {
    zone, value.hour, value.minute, value.second, value.nanosecond};

  if (!in_range(value.month, 1, 12) || !in_range(value.day, 1, 31) ||
      !in_range(value.hour, 0, 24) || !in_range(value.minute, 0, 59) ||
      !in_range(value.second, 0, 59) || value.nanosecond > 999999999) {
    return end_write(EXESS_BAD_VALUE, buf_size, buf, 0);
  }

  // Write date
  ExessResult dr = exess_write_date(date, buf_size, buf);
  if (dr.status) {
    return end_write(dr.status, buf_size, buf, dr.count);
  }

  // Write time delimiter
  size_t o = dr.count + write_char('T', buf_size, buf, dr.count);

  // Write time with timezone
  const ExessResult tr = write_time(time, buf_size, buf, o);

  return end_write(tr.status, buf_size, buf, o + tr.count);
}
