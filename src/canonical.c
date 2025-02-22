// Copyright 2019-2025 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#include "read_utils.h"
#include "result.h"
#include "string_utils.h"
#include "write_utils.h"

#include <exess/exess.h>

#include <stdbool.h>
#include <stddef.h>

/// Return true iff `c` is "0"
static inline bool
is_zero(const int c)
{
  return c == '0';
}

/// Return true iff `c` is "."
static inline bool
is_point(const int c)
{
  return c == '.';
}

// Scan forwards as long as `pred` returns true for characters
static inline size_t
scan(bool (*pred)(const int), const char* const str, size_t i)
{
  while (pred(str[i])) {
    ++i;
  }

  return i;
}

// Skip the next character if `pred` returns true for it
static inline size_t
skip(bool (*pred)(const int), const char* const str, const size_t i)
{
  return i + (pred(str[i]) ? 1 : 0);
}

static ExessVariableResult
write_decimal(const char* const str, const size_t buf_size, char* const buf)
{
  const size_t sign    = scan(is_space, str, 0);   // Sign
  const size_t leading = skip(is_sign, str, sign); // First digit
  if (str[leading] != '.' && !is_digit(str[leading])) {
    return vresult(EXESS_EXPECTED_DIGIT, sign, 0);
  }

  const size_t first = scan(is_zero, str, leading); // First non-zero
  const size_t point = scan(is_digit, str, first);  // Decimal point
  const size_t end =
    scan(is_digit, str, skip(is_point, str, point)); // Last digit

  const ExessResult r = end_read(EXESS_SUCCESS, str, end);
  if (r.status) {
    return vresult(r.status, r.count, 0);
  }

  // Ignore trailing zeros
  size_t last = end;
  if (str[point] == '.') {
    while (str[last - 1] == '0') {
      --last;
    }
  }

  // Add leading sign only if the number is negative
  size_t o = 0;
  if (str[sign] == '-') {
    o += write_char('-', buf_size, buf, o);
  }

  // Handle zero as a special case (no non-zero digits to copy)
  if (first == last) {
    o += write_string(3, "0.0", buf_size, buf, o);
    return vresult(EXESS_SUCCESS, end, o);
  }

  // Add leading zero if needed to have at least one digit before the point
  if (str[first] == '.') {
    o += write_char('0', buf_size, buf, o);
  }

  // Add digits
  o += write_string(last - first, str + first, buf_size, buf, o);

  if (str[point] != '.') {
    // Add missing decimal suffix
    o += write_string(2, ".0", buf_size, buf, o);
  } else if (point == last - 1) {
    // Add missing trailing zero after point
    o += write_char('0', buf_size, buf, o);
  }

  const ExessResult w = end_write(EXESS_SUCCESS, buf_size, buf, o);
  return vresult(w.status, end, o);
}

static ExessVariableResult
write_integer(const ExessDatatype datatype,
              const char* const   str,
              const size_t        buf_size,
              char* const         buf)
{
  const size_t sign = scan(is_space, str, 0); // Sign

  if ((str[sign] == '-' && (datatype == EXESS_NON_NEGATIVE_INTEGER ||
                            datatype == EXESS_POSITIVE_INTEGER)) ||
      (str[sign] == '+' && (datatype == EXESS_NON_POSITIVE_INTEGER ||
                            datatype == EXESS_NEGATIVE_INTEGER)) ||
      (str[sign] != '-' && (datatype == EXESS_NEGATIVE_INTEGER))) {
    return vresult(EXESS_BAD_VALUE, sign, 0);
  }

  const size_t leading = skip(is_sign, str, sign); // First digit
  if (!is_digit(str[leading])) {
    return vresult(EXESS_EXPECTED_DIGIT, leading, 0);
  }

  const size_t first = scan(is_zero, str, leading); // First non-zero
  const size_t last  = scan(is_digit, str, first);  // Last digit

  // Handle zero as a special case (no non-zero digits to copy)
  size_t o = 0;
  if (first == last) {
    if (datatype == EXESS_NEGATIVE_INTEGER ||
        datatype == EXESS_POSITIVE_INTEGER) {
      return vresult(EXESS_BAD_VALUE, sign, 0);
    }

    o += write_char('0', buf_size, buf, o);
    return vresult(EXESS_SUCCESS, first, o);
  }

  // Add leading sign only if the number is negative
  ExessStatus st = EXESS_SUCCESS;
  if (str[sign] == '-') {
    st = (datatype == EXESS_NON_NEGATIVE_INTEGER ||
          datatype == EXESS_POSITIVE_INTEGER)
           ? EXESS_BAD_VALUE
           : EXESS_SUCCESS;

    o += write_char('-', buf_size, buf, o);
  } else if (datatype == EXESS_NON_POSITIVE_INTEGER ||
             datatype == EXESS_NEGATIVE_INTEGER) {
    return vresult(EXESS_BAD_VALUE, first, sign);
  }

  // Add digits
  o += write_string(last - first, str + first, buf_size, buf, o);

  return vresult(st, last, o);
}

static ExessVariableResult
write_date_time(const char* const str, const size_t buf_size, char* const buf)
{
  static const ExessDuration zero = {0, 0, 0};

  ExessDateTime     value = {0, 0, 0, 0, 0, 0, 0, 0};
  const ExessResult r     = exess_read_date_time(&value, str);
  if (r.status) {
    return vresult(r.status, r.count, 0);
  }

  // Wrap 24:00 midnight to 00:00 on the next day
  const ExessResult w = exess_write_date_time(
    (value.hour == 24) ? exess_add_date_time_duration(value, zero) : value,
    buf_size,
    buf);

  return vresult(w.status, r.count, w.count);
}

static ExessVariableResult
write_hex(const char* const str, const size_t buf_size, char* const buf)
{
  size_t i = 0;
  size_t o = 0;

  for (; str[i]; ++i) {
    if (is_hexdig(str[i])) {
      o += write_char(str[i], buf_size, buf, o);
    } else if (!is_space(str[i])) {
      return vresult(EXESS_EXPECTED_HEX, i, o);
    }
  }

  return vresult(
    (o == 0 || o % 2 != 0) ? EXESS_EXPECTED_HEX : EXESS_SUCCESS, i, o);
}

static ExessVariableResult
write_base64(const char* const str, const size_t buf_size, char* const buf)
{
  size_t i = 0;
  size_t o = 0;

  for (; str[i]; ++i) {
    if (is_base64(str[i])) {
      o += write_char(str[i], buf_size, buf, o);
    } else if (!is_space(str[i])) {
      return vresult(EXESS_EXPECTED_BASE64, i, o);
    }
  }

  if (o == 0 || o % 4 != 0) {
    return vresult(EXESS_EXPECTED_BASE64, i, o);
  }

  return vresult(EXESS_SUCCESS, i, o);
}

static ExessVariableResult
write_bounded(const char* const   str,
              const ExessDatatype datatype,
              const size_t        buf_size,
              char* const         buf)
{
  ExessValue value = {false};

  const ExessVariableResult vr =
    exess_read_value(datatype, sizeof(value), &value, str);

  if (vr.status) {
    return vresult(vr.status, vr.read_count, 0U);
  }

  const ExessResult w =
    exess_write_value(datatype, vr.read_count, &value, buf_size, buf);
  return vresult(w.status, vr.read_count, w.count);
}

ExessVariableResult
exess_write_canonical(const char* const   str,
                      const ExessDatatype datatype,
                      const size_t        buf_size,
                      char* const         buf)
{
  const ExessVariableResult r =
    (datatype == EXESS_DECIMAL) ? write_decimal(str, buf_size, buf)
    : ((datatype == EXESS_INTEGER) ||
       (datatype == EXESS_NON_POSITIVE_INTEGER) ||
       (datatype == EXESS_NEGATIVE_INTEGER) ||
       (datatype == EXESS_NON_NEGATIVE_INTEGER) ||
       (datatype == EXESS_POSITIVE_INTEGER))
      ? write_integer(datatype, str, buf_size, buf)
    : (datatype == EXESS_DATE_TIME) ? write_date_time(str, buf_size, buf)
    : (datatype == EXESS_HEX)       ? write_hex(str, buf_size, buf)
    : (datatype == EXESS_BASE64)    ? write_base64(str, buf_size, buf)
                                 : write_bounded(str, datatype, buf_size, buf);

  const ExessResult w = end_write(r.status, buf_size, buf, r.write_count);
  return vresult(w.status, r.read_count, w.count);
}
