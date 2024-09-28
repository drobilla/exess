// Copyright 2021-2023 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#ifndef EXESS_EXESS_H
#define EXESS_EXESS_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// EXESS_API must be used to decorate things in the public API
#ifndef EXESS_API
#  if defined(_WIN32) && !defined(EXESS_STATIC) && defined(EXESS_INTERNAL)
#    define EXESS_API __declspec(dllexport)
#  elif defined(_WIN32) && !defined(EXESS_STATIC)
#    define EXESS_API __declspec(dllimport)
#  elif defined(__GNUC__)
#    define EXESS_API __attribute__((visibility("default")))
#  else
#    define EXESS_API
#  endif
#endif

// GCC function attributes
#ifdef __GNUC__
#  define EXESS_PURE_FUNC __attribute__((pure))
#  define EXESS_CONST_FUNC __attribute__((const))
#else
#  define EXESS_PURE_FUNC
#  define EXESS_CONST_FUNC
#endif

// Clang nullability attributes
#if defined(__clang__) && __clang_major__ >= 7
#  define EXESS_NONNULL _Nonnull
#  define EXESS_NULLABLE _Nullable
#else
#  define EXESS_NONNULL
#  define EXESS_NULLABLE
#endif

/// A pure function in the public API that only reads memory
#define EXESS_PURE_API EXESS_API EXESS_PURE_FUNC

/// A const function in the public API that's pure and only reads parameters
#define EXESS_CONST_API EXESS_API EXESS_CONST_FUNC

#ifdef __cplusplus
extern "C" {
#endif

/**
   @defgroup exess Exess C API
   @{
*/

/**
   @defgroup exess_symbols Symbols

   Preprocessor symbols for convenience.

   @{
*/

/// The base URI of XML Schema, `http://www.w3.org/2001/XMLSchema#`
#define EXESS_XSD_URI "http://www.w3.org/2001/XMLSchema#"

/**
   @}
   @defgroup exess_status Status

   Status codes and return values used for error handling.

   Success and various specific errors are reported by an integer status code,
   which can be converted to a string to produce friendly error messages.
   Reading and writing functions return a "result", which has a status code
   along with a count of bytes read or written.

   @{
*/

/// Status code to describe errors or other relevant situations
typedef enum {
  EXESS_SUCCESS,                ///< Success
  EXESS_EXPECTED_END,           ///< Expected end of value
  EXESS_EXPECTED_BOOLEAN,       ///< Expected "false", "true", "0" or "1"
  EXESS_EXPECTED_INTEGER,       ///< Expected an integer value
  EXESS_EXPECTED_DURATION,      ///< Expected a duration starting with 'P'
  EXESS_EXPECTED_SIGN,          ///< Expected '-' or '+'
  EXESS_EXPECTED_DIGIT,         ///< Expected a digit
  EXESS_EXPECTED_COLON,         ///< Expected ':'
  EXESS_EXPECTED_DASH,          ///< Expected '-'
  EXESS_EXPECTED_TIME_SEP,      ///< Expected 'T'
  EXESS_EXPECTED_TIME_TAG,      ///< Expected 'H', 'M', or 'S'
  EXESS_EXPECTED_DATE_TAG,      ///< Expected 'Y', 'M', or 'D'
  EXESS_EXPECTED_HEX,           ///< Expected a hexadecimal character
  EXESS_EXPECTED_BASE64,        ///< Expected a base64 character
  EXESS_BAD_ORDER,              ///< Invalid field order
  EXESS_BAD_VALUE,              ///< Invalid value
  EXESS_OUT_OF_RANGE,           ///< Value out of range for datatype
  EXESS_NO_SPACE,               ///< Insufficient space
  EXESS_WOULD_REDUCE_PRECISION, ///< Precision reducing coercion required
  EXESS_WOULD_ROUND,            ///< Rounding coercion required
  EXESS_WOULD_TRUNCATE,         ///< Truncating coercion required
  EXESS_UNSUPPORTED,            ///< Unsupported value
} ExessStatus;

/**
   Result returned from a read or write function.

   This combines a status code with a byte offset, so it can be used to
   determine how many characters were read or written, or what error occurred
   at what character offset.
*/
typedef struct {
  ExessStatus status; ///< Status code
  size_t      count;  ///< Number of bytes read or written, excluding null
} ExessResult;

/**
   Result returned from a read function for variably-sized values.

   This is like #ExessResult but includes separate read and write counts.
   This allows the caller to know both how many bytes were read from the input
   (for advancing an input cursor), and how many bytes were written to the
   output (to know how large the value is).
*/
typedef struct {
  ExessStatus status;      ///< Status code
  size_t      read_count;  ///< Number of bytes read
  size_t      write_count; ///< Number of bytes written, excluding null
} ExessVariableResult;

/**
   Return a string describing a status code in plain English.

   The returned string is always one sentence, with an uppercase first
   character, and no trailing period.
*/
EXESS_CONST_API const char* EXESS_NONNULL
exess_strerror(ExessStatus status);

/**
   @}
   @defgroup exess_numbers Numbers
   @{
*/

/**
   @defgroup exess_decimal Decimal

   An xsd:decimal is a decimal number of arbitrary precision, but this
   implementation only supports values that fit in a `double`.

   Unlike xsd:double, xsd:decimal is written in numeric form, never in
   scientific notation.  Special infinity and NaN values are not supported.
   Note that the xsd:decimal representation for some numbers is very long, so
   xsd:double may be a better choice for values in a wide range.

   Canonical form has no leading "+" sign, and at most 1 leading or trailing
   zero such that there is at least 1 digit on either side of the decimal
   point, like "12.34", "-1.0", and "0.0".

   Non-canonical form allows a leading "+", any number of leading and trailing
   zeros, any number of digits (including zero) on either side of the point,
   and does not require a decimal point, like "+1", "01", "-.5", "4.", and
   "42".

   @{
*/

/// The maximum length of an xsd:decimal string from exess_write_decimal(), 327
#define EXESS_MAX_DECIMAL_LENGTH 327

/**
   Read an xsd:decimal string after any leading whitespace.

   Values beyond the range of `decimal` will produce `-INF` or `INF`, and
   return an error because these are not valid decimal values.

   @param out Set to the parsed value, or NaN on error.
   @param str String input.
   @return The `count` of characters read, and a `status` code.
*/
EXESS_API ExessResult
exess_read_decimal(double* EXESS_NONNULL out, const char* EXESS_NONNULL str);

/**
   Write a canonical xsd:decimal string.

   @param value Value to write.
   @param buf_size The size of `buf` in bytes.
   @param buf Output buffer, or null to only measure.

   @return The `count` of characters in the output, and `status`
   #EXESS_SUCCESS, or #EXESS_NO_SPACE if the buffer is too small.
*/
EXESS_API ExessResult
exess_write_decimal(double value, size_t buf_size, char* EXESS_NULLABLE buf);

/**
   @}
   @defgroup exess_double Double

   An xsd:double is an IEEE-754 64-bit floating point number, written in
   scientific notation.

   Canonical form has no leading "+" sign, at most 1 leading or trailing zero
   such that there is at least 1 digit on either side of the decimal point, and
   always has an exponent, like "12.34E56", "-1.0E-2", and "-0.0E0".  The
   special values negative infinity, positive infinity, and not-a-number are
   written "-INF", "INF", and "NaN", respectively.

   Non-canonical form allows a leading "+", any number of leading and trailing
   zeros, any number of digits (including zero) on either side of the point,
   and does not require an exponent or decimal point, like "+1E3", "1E+3",
   ".5E3", "4.2", and "42".

   @{
*/

/// The maximum length of a canonical xsd:double string, 24
#define EXESS_MAX_DOUBLE_LENGTH 24

/**
   Read an xsd:double string after any leading whitespace.

   Values beyond the range of `double` will produce `-INF` or `INF`.

   @param out Set to the parsed value, or `NAN` on error.
   @param str String input.
   @return The `count` of characters read, and a `status` code.
*/
EXESS_API ExessResult
exess_read_double(double* EXESS_NONNULL out, const char* EXESS_NONNULL str);

/**
   Write a canonical xsd:double string.

   Any `double` value is supported.  Reading the resulting string with
   exess_read_double() will produce exactly `value`, except the extra bits in
   NaNs are not preserved.

   @param value Value to write.
   @param buf_size The size of `buf` in bytes.
   @param buf Output buffer, or null to only measure.

   @return The `count` of characters in the output and `status` #EXESS_SUCCESS,
   or #EXESS_NO_SPACE if the buffer is too small.
*/
EXESS_API ExessResult
exess_write_double(double value, size_t buf_size, char* EXESS_NULLABLE buf);

/**
   @}
   @defgroup exess_float Float

   An xsd:float is an IEEE-754 32-bit floating point number, written in
   scientific notation.

   The lexical form is the same as xsd:double, the only difference is that the
   value space of xsd:float is smaller.  See @ref exess_double for details.

   @{
*/

/// The maximum length of a canonical xsd:float string, 15
#define EXESS_MAX_FLOAT_LENGTH 15

/**
   Read an xsd:float string after any leading whitespace.

   Values beyond the range of `float` will produce `-INF` or `INF`.

   @param out Set to the parsed value, or `NAN` on error.
   @param str String input.
   @return The `count` of characters read, and a `status` code.
*/
EXESS_API ExessResult
exess_read_float(float* EXESS_NONNULL out, const char* EXESS_NONNULL str);

/**
   Write a canonical xsd:float string.

   Any `float` value is supported.  Reading the resulting string with
   exess_read_float() will produce exactly `value`, except the extra bits in
   NaNs are not preserved.

   @param value Value to write.
   @param buf_size The size of `buf` in bytes.
   @param buf Output buffer, or null to only measure.

   @return The `count` of characters in the output, and `status`
   #EXESS_SUCCESS, or #EXESS_NO_SPACE if the buffer is too small.
*/
EXESS_API ExessResult
exess_write_float(float value, size_t buf_size, char* EXESS_NULLABLE buf);

/**
   @}
   @defgroup exess_boolean Boolean

   An xsd:boolean has only two possible values, canonically written as "false"
   and "true".  The non-canonical forms "0" and "1" are also supported.

   @{
*/

/// The maximum length of a canonical xsd:boolean string, 5
#define EXESS_MAX_BOOLEAN_LENGTH 5

/**
   Read an xsd:boolean string after any leading whitespace.

   @param out Set to the parsed value, or false on error.
   @param str String input.
   @return The `count` of characters read, and a `status` code.
*/
EXESS_API ExessResult
exess_read_boolean(bool* EXESS_NONNULL out, const char* EXESS_NONNULL str);

/**
   Write a canonical xsd:boolean string.

   @param value Value to write.
   @param buf_size The size of `buf` in bytes.
   @param buf Output buffer, or null to only measure.

   @return The `count` of characters in the output, and `status`
   #EXESS_SUCCESS, or #EXESS_NO_SPACE if the buffer is too small.
*/
EXESS_API ExessResult
exess_write_boolean(bool value, size_t buf_size, char* EXESS_NULLABLE buf);

/**
   @}
   @defgroup exess_long Long

   An xsd:long is a signed 64-bit integer, written in decimal.

   Values range from -9223372036854775808 to 9223372036854775807 inclusive.

   Canonical form has no leading "+" sign and no leading zeros (except for the
   number "0"), like "-1", "0", and "1234".

   Non-canonical form allows a leading "+" and any number of leading zeros,
   like "01" and "+0001234".

   @{
*/

/// The maximum length of a canonical xsd:long string, 20
#define EXESS_MAX_LONG_LENGTH 20

/**
   Read an xsd:long string after any leading whitespace.

   @param out Set to the parsed value, or zero on error.
   @param str String input.
   @return The `count` of characters read, and a `status` code.
*/
EXESS_API ExessResult
exess_read_long(int64_t* EXESS_NONNULL out, const char* EXESS_NONNULL str);

/**
   Write a canonical xsd:long string.

   @param value Value to write.
   @param buf_size The size of `buf` in bytes.
   @param buf Output buffer, or null to only measure.

   @return The `count` of characters in the output, and `status`
   #EXESS_SUCCESS, or #EXESS_NO_SPACE if the buffer is too small.
*/
EXESS_API ExessResult
exess_write_long(int64_t value, size_t buf_size, char* EXESS_NULLABLE buf);

/**
   @}
   @defgroup exess_int Int

   An xsd:int is a signed 32-bit integer.

   Values range from -2147483648 to 2147483647 inclusive.

   The lexical form is the same as xsd:long, the only difference is that the
   value space of xsd:int is smaller.  See @ref exess_long for details.

   @{
*/

/// The maximum length of a canonical xsd:int string, 11
#define EXESS_MAX_INT_LENGTH 11

/**
   Read an xsd:int string after any leading whitespace.

   @param out Set to the parsed value, or zero on error.
   @param str String input.
   @return The `count` of characters read, and a `status` code.
*/
EXESS_API ExessResult
exess_read_int(int32_t* EXESS_NONNULL out, const char* EXESS_NONNULL str);

/**
   Write a canonical xsd:int string.

   @param value Value to write.
   @param buf_size The size of `buf` in bytes.
   @param buf Output buffer, or null to only measure.

   @return The `count` of characters in the output, and `status`
   #EXESS_SUCCESS, or #EXESS_NO_SPACE if the buffer is too small.
*/
EXESS_API ExessResult
exess_write_int(int32_t value, size_t buf_size, char* EXESS_NULLABLE buf);

/**
   @}
   @defgroup exess_short Short

   An xsd:short is a signed 16-bit integer.

   Values range from -32768 to 32767 inclusive.

   The lexical form is the same as xsd:long, the only difference is that the
   value space of xsd:short is smaller.  See @ref exess_long for details.

   @{
*/

/// The maximum length of a canonical xsd:short string, 6
#define EXESS_MAX_SHORT_LENGTH 6

/**
   Read an xsd:short string after any leading whitespace.

   @param out Set to the parsed value, or zero on error.
   @param str String input.
   @return The `count` of characters read, and a `status` code.
*/
EXESS_API ExessResult
exess_read_short(int16_t* EXESS_NONNULL out, const char* EXESS_NONNULL str);

/**
   Write a canonical xsd:short string.

   @param value Value to write.
   @param buf_size The size of `buf` in bytes.
   @param buf Output buffer, or null to only measure.

   @return The `count` of characters in the output, and `status`
   #EXESS_SUCCESS, or #EXESS_NO_SPACE if the buffer is too small.
*/
EXESS_API ExessResult
exess_write_short(int16_t value, size_t buf_size, char* EXESS_NULLABLE buf);

/**
   @}
   @defgroup exess_byte Byte

   An xsd:byte is a signed 8-bit integer.

   Values range from -128 to 127 inclusive.

   The lexical form is the same as xsd:long, the only difference is that the
   value space of xsd:byte is smaller.  See @ref exess_long for details.

   @{
*/

/// The maximum length of a canonical xsd:byte string, 4
#define EXESS_MAX_BYTE_LENGTH 4

/**
   Read an xsd:byte string after any leading whitespace.

   @param out Set to the parsed value, or zero on error.
   @param str String input.
   @return The `count` of characters read, and a `status` code.
*/
EXESS_API ExessResult
exess_read_byte(int8_t* EXESS_NONNULL out, const char* EXESS_NONNULL str);

/**
   Write a canonical xsd:byte string.

   @param value Value to write.
   @param buf_size The size of `buf` in bytes.
   @param buf Output buffer, or null to only measure.

   @return The `count` of characters in the output, and `status`
   #EXESS_SUCCESS, or #EXESS_NO_SPACE if the buffer is too small.
*/
EXESS_API ExessResult
exess_write_byte(int8_t value, size_t buf_size, char* EXESS_NULLABLE buf);

/**
   @}
   @defgroup exess_ulong Unsigned Long

   An xsd:unsignedLong is an unsigned 64-bit integer, written in decimal.

   Values range from 0 to 18446744073709551615 inclusive.

   Canonical form has no leading "+" sign and no leading zeros (except for the
   number "0"), like "0", and "1234".

   Non-canonical form allows any number of leading zeros, like "01" and
   "0001234".

   @{
*/

/// The maximum length of a canonical xsd:unsignedLong string, 20
#define EXESS_MAX_ULONG_LENGTH 20

/**
   Read an xsd:unsignedLong string after any leading whitespace.

   @param out Set to the parsed value, or zero on error.
   @param str String input.
   @return The `count` of characters read, and a `status` code.
*/
EXESS_API ExessResult
exess_read_ulong(uint64_t* EXESS_NONNULL out, const char* EXESS_NONNULL str);

/**
   Write a canonical xsd:unsignedLong string.

   @param value Value to write.
   @param buf_size The size of `buf` in bytes.
   @param buf Output buffer, or null to only measure.

   @return The `count` of characters in the output, and `status`
   #EXESS_SUCCESS, or #EXESS_NO_SPACE if the buffer is too small.
*/
EXESS_API ExessResult
exess_write_ulong(uint64_t value, size_t buf_size, char* EXESS_NULLABLE buf);

/**
   @}
   @defgroup exess_uint Unsigned Int

   An xsd:unsignedInt is an unsigned 32-bit integer.

   Values range from 0 to 4294967295 inclusive.

   The lexical form is the same as xsd:unsignedLong, the only difference is
   that the value space of xsd:unsignedInt is smaller.  See @ref exess_ulong
   for details.

   @{
*/

/// The maximum length of a canonical xsd:unsignedInt string, 10
#define EXESS_MAX_UINT_LENGTH 10

/**
   Read an xsd:unsignedInt string after any leading whitespace.

   @param out Set to the parsed value, or zero on error.
   @param str String input.
   @return The `count` of characters read, and a `status` code.
*/
EXESS_API ExessResult
exess_read_uint(uint32_t* EXESS_NONNULL out, const char* EXESS_NONNULL str);

/**
   Write a canonical xsd:unsignedInt string.

   @param value Value to write.
   @param buf_size The size of `buf` in bytes.
   @param buf Output buffer, or null to only measure.

   @return The `count` of characters in the output, and `status`
   #EXESS_SUCCESS, or #EXESS_NO_SPACE if the buffer is too small.
*/
EXESS_API ExessResult
exess_write_uint(uint32_t value, size_t buf_size, char* EXESS_NULLABLE buf);

/**
   @}
   @defgroup exess_ushort Unsigned Short

   An xsd:unsignedShort is an unsigned 16-bit integer.

   Values range from 0 to 65535 inclusive.

   The lexical form is the same as xsd:unsignedLong, the only difference is
   that the value space of xsd:unsignedShort is smaller.  See @ref exess_ulong
   for details.

   @{
*/

/// The maximum length of a canonical xsd:unsignedShort string, 5
#define EXESS_MAX_USHORT_LENGTH 5

/**
   Read an xsd:unsignedShort string after any leading whitespace.

   @param out Set to the parsed value, or zero on error.
   @param str String input.
   @return The `count` of characters read, and a `status` code.
*/
EXESS_API ExessResult
exess_read_ushort(uint16_t* EXESS_NONNULL out, const char* EXESS_NONNULL str);

/**
   Write a canonical xsd:unsignedShort string.

   @param value Value to write.
   @param buf_size The size of `buf` in bytes.
   @param buf Output buffer, or null to only measure.

   @return The `count` of characters in the output, and `status`
   #EXESS_SUCCESS, or #EXESS_NO_SPACE if the buffer is too small.
*/
EXESS_API ExessResult
exess_write_ushort(uint16_t value, size_t buf_size, char* EXESS_NULLABLE buf);

/**
   @}
   @defgroup exess_ubyte Unsigned Byte

   An xsd:unsignedByte is an unsigned 8-bit integer.  Values range from 0 to
   255 inclusive.

   The lexical form is the same as xsd:unsignedLong, the only difference is
   that the value space of xsd:unsignedByte is smaller.  See @ref exess_ulong
   for details.

   @{
*/

/// The maximum length of a canonical xsd:unsignedByte string, 3
#define EXESS_MAX_UBYTE_LENGTH 3

/**
   Read an xsd:unsignedByte string after any leading whitespace.

   @param out Set to the parsed value, or zero on error.
   @param str String input.
   @return The `count` of characters read, and a `status` code.
*/
EXESS_API ExessResult
exess_read_ubyte(uint8_t* EXESS_NONNULL out, const char* EXESS_NONNULL str);

/**
   Write a canonical xsd:unsignedByte string.

   @param value Value to write.
   @param buf_size The size of `buf` in bytes.
   @param buf Output buffer, or null to only measure.

   @return The `count` of characters in the output, and `status`
   #EXESS_SUCCESS, or #EXESS_NO_SPACE if the buffer is too small.
*/
EXESS_API ExessResult
exess_write_ubyte(uint8_t value, size_t buf_size, char* EXESS_NULLABLE buf);

/**
   @}
   @}
   @defgroup exess_time_and_date Time and Date
   @{
*/

/**
   @defgroup exess_duration Duration

   An xsd:duration is a positive or negative duration of time, written in ISO
   8601 format like "PnYnMnDTnHnMnS" where each "n" is a number and fields may
   be omitted if they are zero.

   All numbers must be integers, except for seconds which may be a decimal.  If
   seconds is a decimal, then at least one digit must follow the decimal point.
   A negative duration is written with "-" as the first character, for example
   "-P60D".

   Canonical form omits all zero fields and writes no leading or trailing
   zeros, except for the zero duration which is written "P0Y", for example
   "P1DT2H", "PT30M", or "PT4.5S".

   Non-canonical form allows zero fields, leading zeros, and for seconds to be
   written as a decimal even if it's integer, for example "P06D", "PT7.0S", or
   "P0Y0M01DT06H00M00S".

   @{
*/

/// The maximum length of an xsd:duration string from exess_write_duration(), 41
#define EXESS_MAX_DURATION_LENGTH 41

/**
   A duration of time (xsd:duration value).

   To save space and to simplify arithmetic, this representation only stores
   two values: integer months, and decimal seconds (to nanosecond precision).
   These values are converted to and from the other fields during writing and
   reading.  Years and months are stored as months, and days, hours, minutes,
   and seconds are stored as seconds.

   The sign of all members must match, so a negative duration has all
   non-positive members, and a positive duration has all non-negative members.
*/
typedef struct {
  int32_t months;      ///< Number of months
  int32_t seconds;     ///< Number of seconds
  int32_t nanoseconds; ///< Number of nanoseconds
} ExessDuration;

/**
   Compare two durations.

   Note that xsd:duration literals are not totally ordered in general, since
   they can include all fields of a date, and the relation between those is not
   always constant (such as the number of days in a month).  The ExessDuration
   representation, however, is normalized in a way that avoids this problem.

   A duration is less than another if it's a shorter duration of time.

   @return -1, 0, or 1 if `lhs` is less than, equal to, or greater than `rhs`,
   respectively.
*/
EXESS_CONST_API int
exess_duration_compare(ExessDuration lhs, ExessDuration rhs);

/**
   Read an xsd:duration string after any leading whitespace.

   @param out Set to the parsed value, or zero on error.
   @param str String input.
   @return The `count` of characters read, and a `status` code.
*/
EXESS_API ExessResult
exess_read_duration(ExessDuration* EXESS_NONNULL out,
                    const char* EXESS_NONNULL    str);

/**
   Write a canonical xsd:duration string.

   @param value Value to write.
   @param buf_size The size of `buf` in bytes.
   @param buf Output buffer, or null to only measure.

   @return #EXESS_SUCCESS on success, #EXESS_NO_SPACE if the buffer is too
   small, or #EXESS_BAD_VALUE if the value is invalid.
*/
EXESS_API ExessResult
exess_write_duration(ExessDuration        value,
                     size_t               buf_size,
                     char* EXESS_NULLABLE buf);
/**
   @}
   @defgroup exess_datetime Datetime

   An xsd:datetime is a date and time in either UTC or local time.

   Strings have the form YYYY-MM-DDTHH:MM:SS with at least 4 year digits
   (negative or positive), and all other fields positive two-digit integers
   except seconds which may be a decimal, for example "2001-02-03T12:13:14.56".
   Nanosecond precision is supported.

   A local datetime has no suffix, a datetime with a time zone is always in
   UTC, and is written with a "Z" suffix, for example 2001-02-03T12:13:14Z.

   Canonical form only includes a decimal point if the number of seconds is not
   an integer.

   This implementation supports up to nanosecond resolution.

   @{
*/

/// The maximum length of an xsd:dateTime string from exess_write_datetime(), 32
#define EXESS_MAX_DATETIME_LENGTH 32

/**
   A date and time (xsd:dateTime value).

   This representation follows the syntax, except the UTC flag is stored
   between the date and time for more efficient packing.
*/
typedef struct {
  int16_t  year;       ///< Year: any positive or negative value
  uint8_t  month;      ///< Month: [1, 12]
  uint8_t  day;        ///< Day: [1, 31]
  uint8_t  is_utc;     ///< True if this is UTC (not local) time
  uint8_t  hour;       ///< Hour: [0, 23]
  uint8_t  minute;     ///< Minute: [0, 59]
  uint8_t  second;     ///< Second: [0, 59]
  uint32_t nanosecond; ///< Nanosecond: [0, 999999999]
} ExessDateTime;

/**
   Compare two datetimes.

   Note that datetimes are not totally ordered since the order between UTC and
   local times can be indeterminate.  When comparing UTC and local times, if
   there is a difference of more than 14 hours, then the comparison is
   determinate (according to the XSD specification).  Otherwise, this function
   will arbitrarily order the local time first.

   @return -1, 0, or 1 if `lhs` is less than, equal to, or greater than `rhs`,
   respectively.
*/
EXESS_CONST_API int
exess_datetime_compare(ExessDateTime lhs, ExessDateTime rhs);

/**
   Add a duration to a datetime.

   This advances or rewinds the datetime by the given duration, depending on
   whether the duration is positive or negative.

   If underflow or overflow occur, then this will return an infinite value.  A
   positive infinity has all fields at maximum, and a negative infinity has all
   fields at minimum, except `is_utc` which is preserved from the input (so
   infinities are comparable with the values they came from).  Since 0 and 255
   are never valid months, these can be tested for by checking if the year and
   month are `INT16_MIN` and 0, or `INT16_MAX` and `INT8_MAX`.

   @return `s + d`, or an infinite past or infinite future if underflow or
   overflow occurs.
*/
EXESS_CONST_API ExessDateTime
exess_add_datetime_duration(ExessDateTime s, ExessDuration d);

/**
   Read an xsd:dateTime string after any leading whitespace.

   @param out Set to the parsed value, or zero on error.
   @param str String input.
   @return The `count` of characters read, and a `status` code.
*/
EXESS_API ExessResult
exess_read_datetime(ExessDateTime* EXESS_NONNULL out,
                    const char* EXESS_NONNULL    str);

/**
   Write a canonical xsd:datetime string.

   @param value Value to write.
   @param buf_size The size of `buf` in bytes.
   @param buf Output buffer, or null to only measure.

   @return #EXESS_SUCCESS on success, #EXESS_NO_SPACE if the buffer is too
   small, or #EXESS_BAD_VALUE if the value is invalid.
*/
EXESS_API ExessResult
exess_write_datetime(ExessDateTime        value,
                     size_t               buf_size,
                     char* EXESS_NULLABLE buf);

/**
   @}
   @defgroup exess_timezone Timezones

   Date and time values can have a timezone qualifier suffix.  Note that
   timezone is not a datatype, one only exists as a part of another value.

   Canonical form starts with a sign, followed by two-digit hour and minute
   offsets separated by a colon, like "-06:00" and "+02:30".  The zero offset,
   UTC, is written "Z".

   Non-canonical form also allows writing UTC as "-00:00" or "+00:00".

   This implementation only supports a resolution of 15 minutes, that is, only
   offsets at 0, 15, 30, and 45 minutes within an hour.

   @{
*/

/**
   A time zone offset in quarter hours.

   This is stored in a single byte for compactness in other structures.  Valid
   values are from -56 to 56 inclusive.
*/
typedef int8_t ExessTimezone;

/// Sentinel value for local time (127)
#define EXESS_LOCAL INT8_MAX

/// Sentinel value for UTC time (0)
#define EXESS_UTC 0U

/**
   Construct a time zone offset from hours and minutes.

   This is a convenience constructor that handles the conversion from hours and
   minutes to the quarter-hour offset used in exess.  The sign of both values
   must be the same.  Hours can be from -14 to 14 inclusive, and minutes can
   only be -45, -30, -15, 0, 15, 30, or 45.

   @return A time zone offset in quarter hours, or #EXESS_LOCAL if the
   parameters are invalid or not supported.
*/
EXESS_CONST_API ExessTimezone
exess_timezone(int8_t hours, int8_t minutes);

/**
   @}
   @defgroup exess_date Date
   An xsd:date is a year, month, and day, with optional timezone.
   @{
*/

/// The maximum length of an xsd:date string from exess_write_date(), 18
#define EXESS_MAX_DATE_LENGTH 18

/// Date (xsd:date)
typedef struct {
  int16_t       year;  ///< Year
  uint8_t       month; ///< Month: [1, 12]
  uint8_t       day;   ///< Day: [1, 31]
  ExessTimezone zone;  ///< Time zone
} ExessDate;

/**
   Compare two dates.

   Note that comparison of dates is not always determinate.  The comparison of
   two dates works the same way as the comparison of two datetimes with
   equivalent times, except adjusted according to the timezone if necessary.
   See exess_datetime_compare() for details.

   @return -1, 0, or 1 if `lhs` is less than, equal to, or greater than `rhs`,
   respectively.
*/
EXESS_CONST_API int
exess_date_compare(ExessDate lhs, ExessDate rhs);

/**
   Read an xsd:date string after any leading whitespace.

   @param out Set to the parsed value, or zero on error.
   @param str String input.
   @return The `count` of characters read, and a `status` code.
*/
EXESS_API ExessResult
exess_read_date(ExessDate* EXESS_NONNULL out, const char* EXESS_NONNULL str);

/**
   Write a canonical xsd:date string.

   The output is always in canonical form, like `2001-04-12` or
   `-2001-10-26+02:00`.

   @param value Value to write.
   @param buf_size The size of `buf` in bytes.
   @param buf Output buffer, or null to only measure.

   @return #EXESS_SUCCESS on success, #EXESS_NO_SPACE if the buffer is too
   small, or #EXESS_BAD_VALUE if the value is invalid.
*/
EXESS_API ExessResult
exess_write_date(ExessDate value, size_t buf_size, char* EXESS_NULLABLE buf);

/**
   @}
   @defgroup exess_time Time

   An xsd:time is a time of day, with optional timezone.

   @{
*/

/// The maximum length of an xsd:time string from exess_write_time(), 24
#define EXESS_MAX_TIME_LENGTH 24

/// Time (xsd:time)
typedef struct {
  ExessTimezone zone;       ///< Time zone
  uint8_t       hour;       ///< Hour: [0, 23]
  uint8_t       minute;     ///< Minute: [0, 59]
  uint8_t       second;     ///< Second: [0, 59]
  uint32_t      nanosecond; ///< Nanosecond: [0, 999999999]
} ExessTime;

/**
   Compare two times.

   Note that comparison of times is not always determinate.  The comparison of
   two times works the same way as the comparison of two datetimes with an
   arbitrary date, see exess_datetime_compare() for details.

   @return -1, 0, or 1 if `lhs` is less than, equal to, or greater than `rhs`,
   respectively.
*/
EXESS_CONST_API int
exess_time_compare(ExessTime lhs, ExessTime rhs);

/**
   Read an xsd:time string after any leading whitespace.

   @param out Set to the parsed value, or zero on error.
   @param str String input.
   @return The `count` of characters read, and a `status` code.
*/
EXESS_API ExessResult
exess_read_time(ExessTime* EXESS_NONNULL out, const char* EXESS_NONNULL str);

/**
   Write a canonical xsd:time string.

   The output is always in canonical form, like "12:15" or "02:00Z".

   @param value Value to write.
   @param buf_size The size of `buf` in bytes.
   @param buf Output buffer, or null to only measure.

   @return #EXESS_SUCCESS on success, #EXESS_NO_SPACE if the buffer is too
   small, or #EXESS_BAD_VALUE if the value is invalid.
*/
EXESS_API ExessResult
exess_write_time(ExessTime value, size_t buf_size, char* EXESS_NULLABLE buf);

/**
   @}
   @}
   @defgroup exess_binary Binary
   @{
*/

/**
   @defgroup exess_base64 Base64
   An xsd:base64Binary is arbitrary binary data in base64 encoding.
   @{
*/

/**
   Return the maximum number of bytes required to decode `length` bytes of
   base64.

   The returned value is an upper bound which is only exact for canonical
   strings.

   @param length The number of input (text) bytes to decode.
   @return The size of a decoded value in bytes.
*/
EXESS_CONST_API size_t
exess_base64_decoded_size(size_t length);

/**
   Read a binary value from a base64 string.

   Canonical syntax is a multiple of 4 base64 characters, with either 1 or 2
   trailing "=" characters as necessary, like "Zm9vYg==", with no whitespace.
   All whitespace is skipped when reading.

   The caller must allocate a large enough buffer to read the value, otherwise
   an #EXESS_NO_SPACE error will be returned.  The required space can be
   calculated with exess_base64_decoded_size().

   When this is called, `out` must point to a buffer of at least `out_size`
   bytes.  The returned result contains the exact size of the decoded data,
   which may be smaller than `out_size`.  Only these first bytes are written,
   the rest of the buffer is not modified.

   @param out_size The size of `out` in bytes.
   @param out Buffer where the decoded binary data will be written.
   @param str String to parse.

   @return The `read_count` of characters read, `write_count` of bytes written,
   and a `status` code.
*/
EXESS_API ExessVariableResult
exess_read_base64(size_t                    out_size,
                  void* EXESS_NONNULL       out,
                  const char* EXESS_NONNULL str);

/**
   Write a canonical xsd:base64Binary string.

   The data is always written in canonical form, as a multiple of 4 characters
   with no whitespace and 1 or 2 trailing "=" characters as padding if
   necessary.

   @param data_size The size of `data` in bytes.
   @param data Data to write to a string.
   @param buf_size The size of `buf` in bytes.
   @param buf Output buffer, or null to only measure.

   @return The `count` of characters in the output, and `status`
   #EXESS_SUCCESS, or #EXESS_NO_SPACE if the buffer is too small.
*/
EXESS_API ExessResult
exess_write_base64(size_t                    data_size,
                   const void* EXESS_NONNULL data,
                   size_t                    buf_size,
                   char* EXESS_NULLABLE      buf);

/**
   @}
   @defgroup exess_hex Hex
   An xsd:hexBinary is arbitrary binary data in hexadecimal encoding.
   @{
*/

/**
   Return the maximum number of bytes required to decode `length` bytes of hex.

   The returned value is an upper bound which is only exact for canonical
   strings.

   @param length The number of input (text) bytes to decode.
   @return The size of a decoded value in bytes.
*/
EXESS_CONST_API size_t
exess_hex_decoded_size(size_t length);

/**
   Read a binary value from a hex string.

   Canonical syntax is an even number of uppercase hexadecimal digits with no
   whitespace, like "666F6F".  Lowercase hexadecimal is also supported, and all
   whitespace is skipped when reading.

   The caller must allocate a large enough buffer to read the value, otherwise
   an #EXESS_NO_SPACE error will be returned.  The required space can be
   calculated with exess_hex_decoded_size().

   When this is called, `out` must point to a buffer of at least `out_size`
   bytes.  The returned result contains the exact size of the decoded data,
   which may be smaller than `out_size`.  Only these first bytes are written,
   the rest of the buffer is not modified.

   @param out_size The size of `out` in bytes.
   @param out Buffer where the decoded binary data will be written.
   @param str String to parse.

   @return The `read_count` of characters read, `write_count` of bytes written,
   and a `status` code.
*/
EXESS_API ExessVariableResult
exess_read_hex(size_t                    out_size,
               void* EXESS_NONNULL       out,
               const char* EXESS_NONNULL str);

/**
   Write a canonical xsd:hexBinary string.

   The data is always written in canonical form, as an even number of uppercase
   hexadecimal digits with no whitespace.

   @param data_size The size of `data` in bytes.
   @param data Data to write to a string.
   @param buf_size The size of `buf` in bytes.
   @param buf Output buffer, or null to only measure.

   @return The `count` of characters in the output, and `status`
   #EXESS_SUCCESS, or #EXESS_NO_SPACE if the buffer is too small.
*/
EXESS_API ExessResult
exess_write_hex(size_t                    data_size,
                const void* EXESS_NONNULL data,
                size_t                    buf_size,
                char* EXESS_NULLABLE      buf);

/**
   @}
   @}
   @defgroup exess_datatypes Datatypes
   Runtime integer tags for supported datatypes with conversion to/from URIs.
   @{
*/

/**
   An identifier for a supported datatype.
*/
typedef enum {
  EXESS_NOTHING,              ///< Sentinel for unknown datatypes or errors
  EXESS_BOOLEAN,              ///< xsd:boolean (see @ref exess_boolean)
  EXESS_DECIMAL,              ///< xsd:decimal (see @ref exess_decimal)
  EXESS_DOUBLE,               ///< xsd:double (see @ref exess_double)
  EXESS_FLOAT,                ///< xsd:float (see @ref exess_float)
  EXESS_INTEGER,              ///< xsd:integer (see @ref exess_long)
  EXESS_NON_POSITIVE_INTEGER, ///< xsd:nonPositiveInteger (see @ref exess_long)
  EXESS_NEGATIVE_INTEGER,     ///< xsd:negativeInteger (see @ref exess_long)
  EXESS_LONG,                 ///< xsd:long (see @ref exess_long)
  EXESS_INT,                  ///< xsd:integer (see @ref exess_int)
  EXESS_SHORT,                ///< xsd:short (see @ref exess_short)
  EXESS_BYTE,                 ///< xsd:byte (see @ref exess_byte)
  EXESS_NON_NEGATIVE_INTEGER, ///< xsd:nonNegativeInteger (see @ref exess_ulong)
  EXESS_ULONG,                ///< xsd:unsignedLong (see @ref exess_ulong)
  EXESS_UINT,                 ///< xsd:unsignedInt (see @ref exess_uint)
  EXESS_USHORT,               ///< xsd:unsignedShort (see @ref exess_ushort)
  EXESS_UBYTE,                ///< xsd:unsignedByte (see @ref exess_ubyte)
  EXESS_POSITIVE_INTEGER,     ///< xsd:positiveInteger (see @ref exess_ulong)
  EXESS_DURATION,             ///< xsd:duration (see @ref exess_duration)
  EXESS_DATETIME,             ///< xsd:dateTime (see @ref exess_datetime)
  EXESS_TIME,                 ///< xsd:time (see @ref exess_time)
  EXESS_DATE,                 ///< xsd:date (see @ref exess_date)
  EXESS_HEX,                  ///< xsd:hexBinary (see @ref exess_hex)
  EXESS_BASE64,               ///< xsd:base64Binary (see @ref exess_base64)
} ExessDatatype;

/**
   Return the URI for a supported datatype.

   This only returns URIs that start with
   `http://www.w3.org/2001/XMLSchema#`.

   @param datatype Datatype tag.
   @return The URI of the datatype, or null for #EXESS_NOTHING.
*/
EXESS_CONST_API const char* EXESS_NULLABLE
exess_datatype_uri(ExessDatatype datatype);

/**
   Return the datatype tag for a datatype URI.

   @return A datatype tag, or #EXESS_NOTHING if the URI is not a supported
   datatype.
*/
EXESS_PURE_API ExessDatatype
exess_datatype_from_uri(const char* EXESS_NONNULL uri);

/**
   Return whether a datatype has an upper bound on value sizes.

   This returns true for all datatypes except #EXESS_DECIMAL, #EXESS_INTEGER
   and its half-bounded subtypes #EXESS_NON_POSITIVE_INTEGER,
   #EXESS_NEGATIVE_INTEGER, #EXESS_NON_NEGATIVE_INTEGER, and
   #EXESS_POSITIVE_INTEGER, and the binary types #EXESS_HEX and #EXESS_BASE64.

   For bounded datatypes, the maximum length of the string representation is
   available via exess_max_length(), or as static constants like
   #EXESS_MAX_INT_LENGTH.

   @return True if values of the given datatype have a maximum size.
*/
EXESS_CONST_API bool
exess_datatype_is_bounded(ExessDatatype datatype);

/**
   Return the maximum length of a string with the given datatype.

   For unbounded datatypes, this returns 0.

   @return A string length in bytes, or zero.
*/
EXESS_CONST_API size_t
exess_max_length(ExessDatatype datatype);

/**
   Return the size of a value with the given datatype.

   Note that this is the size of the binary representation, not a string
   length.  For hex and base64Binary, this returns 0, since such values can be
   arbitrarily large.

   @return The size in bytes required by a value, or zero if there is no such
   limit.
*/
EXESS_CONST_API size_t
exess_value_size(ExessDatatype datatype);

/**
   @}
   @defgroup exess_canon Canonical Form
   Rewriting generic strings in canonical form.
   @{
*/

/**
   Rewrite a supported xsd datatype in canonical form.

   @param value Input value string.
   @param datatype Datatype of value.
   @param buf_size The size of `buf` in bytes.
   @param buf Output buffer, or null to only measure.

   @return The `count` of characters in the output, and a `status` code.  The
   status may be an error from reading or writing, but the `count` always
   refers to the number of characters written.
*/
EXESS_API ExessResult
exess_write_canonical(const char* EXESS_NONNULL value,
                      ExessDatatype             datatype,
                      size_t                    buf_size,
                      char* EXESS_NULLABLE      buf);

/**
   @}
   @defgroup exess_value Value
   A generic interface for reading and writing binary values.
   @{
*/

/**
   The maximum size of a supported fixed-size value in bytes.

   This is the size of #ExessValue, which is larger than the size of all the
   supported numeric and time-based datatypes.
*/
#define EXESS_MAX_VALUE_SIZE 16U

/**
   A union that can hold any supported fixed-sized value.

   This can be used as a convenience for allocating a suitable buffer for any
   numeric or time-based value and casting it to the appropriate type.
*/
typedef union {
  bool          as_bool;                       ///< #EXESS_BOOLEAN
  double        as_double;                     ///< #EXESS_DOUBLE
  float         as_float;                      ///< #EXESS_FLOAT
  int64_t       as_long;                       ///< #EXESS_LONG
  int32_t       as_int;                        ///< #EXESS_INT
  int16_t       as_short;                      ///< #EXESS_SHORT
  int8_t        as_byte;                       ///< #EXESS_BYTE
  uint64_t      as_ulong;                      ///< #EXESS_ULONG
  uint32_t      as_uint;                       ///< #EXESS_UINT
  uint16_t      as_ushort;                     ///< #EXESS_USHORT
  uint8_t       as_ubyte;                      ///< #EXESS_UBYTE
  ExessDuration as_duration;                   ///< #EXESS_DURATION
  ExessDateTime as_datetime;                   ///< #EXESS_DATETIME
  ExessTime     as_time;                       ///< #EXESS_TIME
  ExessDate     as_date;                       ///< #EXESS_DATE
  uint8_t       as_blob[EXESS_MAX_VALUE_SIZE]; ///< #EXESS_HEX and #EXESS_BASE64
} ExessValue;

/**
   Coercion flags.

   These values are ORed together to enable different kinds of lossy
   conversion.
*/
typedef enum {
  /**
     Allow coercions that reduce the precision of values.

     This allows coercions that are lossy only in terms of precision, so the
     resulting value is approximately equal to the original value.
     Specifically, this allows coercing double to float.
  */
  EXESS_REDUCE_PRECISION = 1U << 0U,

  /**
     Allow coercions that round to the nearest integer.

     This allows coercing floating point numbers to integers by rounding to the
     nearest integer, with halfway cases rounding towards even (the default
     IEEE-754 rounding order).
  */
  EXESS_ROUND = 1U << 1U,

  /**
     Allow coercions that truncate significant parts of values.

     Specifically, this allows coercing any number to boolean, datetime to
     date, and datetime to time.
  */
  EXESS_TRUNCATE = 1U << 1U,
} ExessCoercion;

/**
   Bitwise OR of #ExessCoercion flags.

   If this is zero, then only lossless coercions will be performed.  A lossless
   coercion is when the value has been perfectly preserved in the target
   datatype, and coercing it back will result in the same value.

   For some datatype combinations this will always be the case, for example
   from short to long.  For others it will depend on the value, for example
   only the numbers 0 and 1 coerce to boolean without loss.
*/
typedef uint32_t ExessCoercions;

/// Readability macro for using no lossy coercions (a zero #ExessCoercions)
#define EXESS_LOSSLESS 0U

/**
   Compare two values.

   @return Less than, equal to, or greater than zero if the left-hand value is
   less than, equal to, or greater than the right-hand value, respectively
   (like `strcmp`).
*/
EXESS_PURE_API int
exess_value_compare(ExessDatatype             lhs_datatype,
                    size_t                    lhs_size,
                    const void* EXESS_NONNULL lhs_value,
                    ExessDatatype             rhs_datatype,
                    size_t                    rhs_size,
                    const void* EXESS_NONNULL rhs_value);

/**
   Coerce a value to another datatype if possible.

   @param coercions Enabled coercion flags.  If this is zero (#EXESS_LOSSLESS),
   then #EXESS_SUCCESS is only returned if the resulting value can be coerced
   back to the original type without any loss of data.  Otherwise, the lossy
   coercions enabled by the set bits will be attempted.

   @param in_datatype The datatype of `in`.
   @param in_size The size of `in` in bytes.
   @param in Input value to coerce.
   @param out_datatype Datatype to convert to.
   @param out_size Size of `out` in bytes.
   @param out Set to the coerced value on success.

   @return #EXESS_SUCCESS on successful conversion, #EXESS_OUT_OF_RANGE if the
   value is outside the range of the target type,
   #EXESS_WOULD_REDUCE_PRECISION, #EXESS_WOULD_ROUND, or #EXESS_WOULD_TRUNCATE
   if the required coercion is not enabled, or #EXESS_UNSUPPORTED if conversion
   between the types is not supported at all.
*/
EXESS_API ExessResult
exess_value_coerce(ExessCoercions            coercions,
                   ExessDatatype             in_datatype,
                   size_t                    in_size,
                   const void* EXESS_NONNULL in,
                   ExessDatatype             out_datatype,
                   size_t                    out_size,
                   void* EXESS_NONNULL       out);

/**
   Read any supported datatype from a string.

   Note that `out` must be suitably aligned for the datatype being read, it
   will be dereferenced directly as a pointer to the value type.  A buffer
   aligned to sizeof(ExessValue) will be suitably aligned for any datatype.

   @param datatype The datatype to read the string as.
   @param out_size The size of `out` in bytes.
   @param out Set to the parsed value on success.
   @param str String input.

   @return The `read_count` from `str`, `write_count` to `out` (both in bytes),
   and a `status` code.
*/
EXESS_API ExessVariableResult
exess_read_value(ExessDatatype             datatype,
                 size_t                    out_size,
                 void* EXESS_NONNULL       out,
                 const char* EXESS_NONNULL str);

/**
   Write any supported datatype to a canonical string.

   Note that `value` must be suitably aligned for the datatype being written,
   it will be dereferenced directly as a pointer to the value type.  A buffer
   aligned to sizeof(ExessValue) will be suitably aligned for any datatype.

   @param datatype The datatype of `value`.
   @param value_size The size of `value` in bytes.
   @param value Value to write.
   @param buf_size The size of `buf` in bytes.
   @param buf Output buffer, or null to only measure.

   @return The `count` of characters in the output, and `status`
   #EXESS_SUCCESS, or #EXESS_NO_SPACE if the buffer is too small.
*/
EXESS_API ExessResult
exess_write_value(ExessDatatype             datatype,
                  size_t                    value_size,
                  const void* EXESS_NONNULL value,
                  size_t                    buf_size,
                  char* EXESS_NULLABLE      buf);

/**
   @}
   @}
*/

#ifdef __cplusplus
} // extern "C"
#endif

#endif // EXESS_EXESS_H
