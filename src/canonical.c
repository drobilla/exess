// Copyright 2019-2023 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#include "read_utils.h"
#include "result.h"
#include "string_utils.h"
#include "write_utils.h"

#include <exess/exess.h>

#include <stdbool.h>
#include <stddef.h>

/// Return true iff `c` is "+" or "-"
static inline bool
is_sign(const int c)
{
  return c == '+' || c == '-';
}

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

static ExessResult
write_decimal(const char* const str, const size_t buf_size, char* const buf)
{
  size_t i = 0;

  const size_t sign    = scan(is_space, str, i);   // Sign
  const size_t leading = skip(is_sign, str, sign); // First digit
  if (str[leading] != '.' && !is_digit(str[leading])) {
    return result(EXESS_EXPECTED_DIGIT, sign);
  }

  const size_t first = scan(is_zero, str, leading); // First non-zero
  const size_t point = scan(is_digit, str, first);  // Decimal point
  size_t last = scan(is_digit, str, skip(is_point, str, point)); // Last digit
  const size_t end = scan(is_space, str, last); // Last non-space

  const ExessStatus st = is_end(str[end]) ? EXESS_SUCCESS : EXESS_EXPECTED_END;

  // Ignore trailing zeros
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
    return result(EXESS_SUCCESS, o);
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

  return result(st, o);
}

static ExessResult
write_integer(const ExessDatatype datatype,
              const char* const   str,
              const size_t        buf_size,
              char* const         buf)
{
  const size_t sign = scan(is_space, str, 0); // Sign
  if (str[sign] == '-' && (datatype == EXESS_NON_NEGATIVE_INTEGER ||
                           datatype == EXESS_POSITIVE_INTEGER)) {
    return result(EXESS_BAD_VALUE, sign);
  }

  const size_t leading = skip(is_sign, str, sign); // First digit
  if (!is_digit(str[leading])) {
    return result(EXESS_EXPECTED_DIGIT, leading);
  }

  const size_t first = scan(is_zero, str, leading); // First non-zero
  const size_t last  = scan(is_digit, str, first);  // Last digit
  const size_t end   = scan(is_space, str, last);   // Last non-space

  ExessStatus st = is_end(str[end]) ? EXESS_SUCCESS : EXESS_EXPECTED_END;

  // Handle zero as a special case (no non-zero digits to copy)
  size_t o = 0;
  if (first == last) {
    if (datatype == EXESS_NEGATIVE_INTEGER ||
        datatype == EXESS_POSITIVE_INTEGER) {
      return result(EXESS_BAD_VALUE, sign);
    }

    o += write_char('0', buf_size, buf, o);
    return result(EXESS_SUCCESS, o);
  }

  // Add leading sign only if the number is negative
  if (str[sign] == '-') {
    st = (datatype == EXESS_NON_NEGATIVE_INTEGER ||
          datatype == EXESS_POSITIVE_INTEGER)
           ? EXESS_BAD_VALUE
           : EXESS_SUCCESS;

    o += write_char('-', buf_size, buf, o);
  } else if (datatype == EXESS_NON_POSITIVE_INTEGER ||
             datatype == EXESS_NEGATIVE_INTEGER) {
    return result(EXESS_BAD_VALUE, sign);
  }

  // Add digits
  o += write_string(last - first, str + first, buf_size, buf, o);

  return result(st, o);
}

static ExessResult
write_hex(const char* const str, const size_t buf_size, char* const buf)
{
  size_t i = 0;
  size_t o = 0;

  for (; str[i]; ++i) {
    if (is_hexdig(str[i])) {
      o += write_char(str[i], buf_size, buf, o);
    } else if (!is_space(str[i])) {
      return result(EXESS_EXPECTED_HEX, o);
    }
  }

  return result((o == 0 || o % 2 != 0) ? EXESS_EXPECTED_HEX : EXESS_SUCCESS, o);
}

static ExessResult
write_base64(const char* const str, const size_t buf_size, char* const buf)
{
  size_t i = 0;
  size_t o = 0;

  for (; str[i]; ++i) {
    if (is_base64(str[i])) {
      o += write_char(str[i], buf_size, buf, o);
    } else if (!is_space(str[i])) {
      return result(EXESS_EXPECTED_BASE64, o);
    }
  }

  if (o == 0 || o % 4 != 0) {
    return result(EXESS_EXPECTED_BASE64, o);
  }

  return result(EXESS_SUCCESS, o);
}

static ExessResult
write_bounded(const char* const   str,
              const ExessDatatype datatype,
              const size_t        buf_size,
              char* const         buf)
{
  ExessValue value = {false};

  const ExessVariableResult vr =
    exess_read_value(datatype, sizeof(value), &value, str);

  return vr.status
           ? result(vr.status, 0U)
           : exess_write_value(datatype, vr.read_count, &value, buf_size, buf);
}

ExessResult
exess_write_canonical(const char* const   str,
                      const ExessDatatype datatype,
                      const size_t        buf_size,
                      char* const         buf)
{
  const ExessResult r =
    (datatype == EXESS_DECIMAL) ? write_decimal(str, buf_size, buf)
    : ((datatype == EXESS_INTEGER) ||
       (datatype == EXESS_NON_POSITIVE_INTEGER) ||
       (datatype == EXESS_NEGATIVE_INTEGER) ||
       (datatype == EXESS_NON_NEGATIVE_INTEGER) ||
       (datatype == EXESS_POSITIVE_INTEGER))
      ? write_integer(datatype, str, buf_size, buf)
    : (datatype == EXESS_HEX)    ? write_hex(str, buf_size, buf)
    : (datatype == EXESS_BASE64) ? write_base64(str, buf_size, buf)
                                 : write_bounded(str, datatype, buf_size, buf);

  return end_write(r.status, buf_size, buf, r.count);
}
