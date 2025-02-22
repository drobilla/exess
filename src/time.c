// Copyright 2019-2025 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#include "read_utils.h"
#include "result.h"
#include "string_utils.h"
#include "time_utils.h"
#include "timezone.h"
#include "write_utils.h"

#include <exess/exess.h>

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

int
exess_compare_time(const ExessTime lhs, const ExessTime rhs)
{
  const ExessDateTime lhs_datetime = {
    1972, 12U, 31U, lhs.zone, lhs.hour, lhs.minute, lhs.second, lhs.nanosecond};

  const ExessDateTime rhs_datetime = {
    1972, 12U, 31U, rhs.zone, rhs.hour, rhs.minute, rhs.second, rhs.nanosecond};

  return exess_compare_date_time(lhs_datetime, rhs_datetime);
}

ExessResult
read_nanoseconds(uint32_t* const out, const char* const str)
{
  // Read available digits
  size_t i = 0U;
  for (; i < 9U && is_digit(str[i]); ++i) {
    *out = (*out * 10U) + (uint32_t)(str[i] - '0');
  }

  if (i == 9U && is_digit(str[i])) {
    return result(EXESS_OUT_OF_RANGE, i);
  }

  // "Read" imagined trailing zeros to reach 9 digit magnitude
  for (size_t j = i; j < 9U; ++j) {
    *out *= 10U;
  }

  return result(i > 0 ? EXESS_SUCCESS : EXESS_EXPECTED_DIGIT, i);
}

ExessResult
read_time(ExessTime* const out, const char* const str)
{
  // Read hour
  size_t      i = 0;
  ExessResult r = read_two_digit_number(&out->hour, 0, 24, str + i);
  if (r.status) {
    return result(r.status, i + r.count);
  }

  const bool midnight = out->hour == 24;

  // Read hour-minute delimiter
  i += r.count;
  if (str[i] != ':') {
    return result(EXESS_EXPECTED_COLON, i);
  }

  // Read minute
  ++i;
  r = read_two_digit_number(&out->minute, 0, midnight ? 0 : 59, str + i);
  if (r.status) {
    return result(r.status, i + r.count);
  }

  // Read minute-second delimiter
  i += r.count;
  if (str[i] != ':') {
    return result(EXESS_EXPECTED_COLON, i);
  }

  // Read second
  ++i;
  r = read_two_digit_number(&out->second, 0, midnight ? 0 : 59, str + i);
  i += r.count;
  if (r.status) {
    return result(r.status, i);
  }

  // Read nanoseconds if present
  if (str[i] == '.') {
    ++i;
    r = read_nanoseconds(&out->nanosecond, str + i);
    i += r.count;
  }
  if (r.status) {
    return result(r.status, i);
  }

  // Ensure midnight has zero nanoseconds
  if (midnight && out->nanosecond) {
    return result(EXESS_OUT_OF_RANGE, i);
  }

  // Read timezone if present
  if (!is_end(str[i])) {
    r = read_timezone(&out->zone, str + i);
    i += r.count;
  } else {
    out->zone = EXESS_LOCAL;
  }

  return end_read(r.status, str, i);
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

size_t
write_nanoseconds(const uint32_t nanosecond,
                  const size_t   buf_size,
                  char* const    buf,
                  const size_t   i)
{
  assert(nanosecond <= 999999999);

  if (nanosecond == 0) {
    return 0;
  }

  char frac_digits[10] = {'0', '0', '0', '0', '0', '0', '0', '0', '0', 0};

  // Write digits right to left, but replace trailing zeros with null
  uint32_t remaining     = nanosecond;
  uint32_t n_trailing    = 0;
  bool     wrote_nonzero = false;
  for (uint32_t j = 0; remaining > 0; ++j) {
    const char digit = (char)('0' + (remaining % 10));
    if (!wrote_nonzero && digit == '0') {
      frac_digits[8 - j] = '\0';
      ++n_trailing;
    } else {
      frac_digits[8 - j] = digit;
    }

    wrote_nonzero = wrote_nonzero || digit != '0';
    remaining /= 10;
  }

  size_t n = write_char('.', buf_size, buf, i);

  n += write_string(9 - n_trailing, frac_digits, buf_size, buf, i + n);

  return n;
}

ExessResult
write_time(const ExessTime value,
           const size_t    buf_size,
           char* const     buf,
           const size_t    offset)
{
  if (value.hour > 24 || value.minute > 59 || value.second > 59 ||
      value.nanosecond > 999999999) {
    return result(EXESS_BAD_VALUE, 0);
  }

  size_t o = offset;

  // Write integral hour, minute, and second
  o += write_two_digit_number(value.hour, buf_size, buf, o);
  o += write_char(':', buf_size, buf, o);
  o += write_two_digit_number(value.minute, buf_size, buf, o);
  o += write_char(':', buf_size, buf, o);
  o += write_two_digit_number(value.second, buf_size, buf, o);
  o += write_nanoseconds(value.nanosecond, buf_size, buf, o);

  const ExessResult r = write_timezone(value.zone, buf_size, buf, o);

  return result(r.status, o - offset + r.count);
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
