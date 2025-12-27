// Copyright 2021-2025 David Robillard <d@drobilla.net>
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
#  define EXESS_UNSPECIFIED _Null_unspecified
#else
#  define EXESS_NONNULL
#  define EXESS_NULLABLE
#  define EXESS_UNSPECIFIED
#endif

/// A pure function in the public API that only reads memory
#define EXESS_PURE_API EXESS_API EXESS_PURE_FUNC

/// A const function in the public API that's pure and only reads parameters
#define EXESS_CONST_API EXESS_API EXESS_CONST_FUNC

/// A static constant expression usable at compile time
#if ((defined(__STDC_VERSION__) && __STDC_VERSION__ >= 202311L) || \
     (defined(__cplusplus) && __cplusplus >= 201103L))
#  define EXESS_CONSTEXPR constexpr
#else
#  define EXESS_CONSTEXPR const
#endif

// Suppress warnings for unused static constants in this header
#if defined(__GNUC__) && __GNUC__ >= 7
_Pragma("GCC diagnostic push")
_Pragma("GCC diagnostic ignored \"-Wunused-const-variable\"")
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
   @defgroup exess Exess C API
   @{
*/

/**
   @defgroup exess_symbols Symbols
   Basic preprocessor symbols and enumerations.
   @{
*/

/// The base URI of XML Schema
#define EXESS_XSD_URI "http://www.w3.org/2001/XMLSchema#"

/**
   The result of comparing two values.

   This follows the usual strcmp() convention, but includes a distinction
   between comparable values that are strictly less/greater than one another,
   and incomparable values that are arbitrarily chosen to be less/greater than
   one another by this implementation.
*/
typedef enum {
  EXESS_ORDER_STRICTLY_LESS    = -2, ///< Comparable, first is lesser
  EXESS_ORDER_MAYBE_LESS       = -1, ///< Incomparable, first may be lesser
  EXESS_ORDER_EQUAL            = 0,  ///< Equal values
  EXESS_ORDER_MAYBE_GREATER    = 1,  ///< Incomparable, first may be greater
  EXESS_ORDER_STRICTLY_GREATER = 2,  ///< Comparable, first is greater
} ExessOrder;

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
  EXESS_SUCCESS,             ///< Success
  EXESS_LOSS,                ///< Lossy operation
  EXESS_EXPECTED_BOOLEAN,    ///< Expected "false", "true", "0" or "1"
  EXESS_EXPECTED_INTEGER,    ///< Expected an integer value
  EXESS_EXPECTED_DURATION,   ///< Expected a duration starting with 'P'
  EXESS_EXPECTED_SIGN,       ///< Expected '-' or '+'
  EXESS_EXPECTED_DIGIT,      ///< Expected a digit
  EXESS_EXPECTED_ZERO,       ///< Expected '0'
  EXESS_EXPECTED_COLON,      ///< Expected ':'
  EXESS_EXPECTED_DASH,       ///< Expected '-'
  EXESS_EXPECTED_TIME_SEP,   ///< Expected 'T'
  EXESS_EXPECTED_TIME_TAG,   ///< Expected 'H', 'M', or 'S'
  EXESS_EXPECTED_DATE_TAG,   ///< Expected 'Y', 'M', or 'D'
  EXESS_EXPECTED_SECOND_TAG, ///< Expected 'S'
  EXESS_EXPECTED_HEX,        ///< Expected a hexadecimal character
  EXESS_EXPECTED_BASE64,     ///< Expected a base64 character
  EXESS_BAD_ORDER,           ///< Invalid field order
  EXESS_BAD_VALUE,           ///< Invalid value
  EXESS_OUT_OF_RANGE,        ///< Value out of range for datatype
  EXESS_NO_SPACE,            ///< Insufficient space
  EXESS_UNSUPPORTED,         ///< Unsupported value
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
   Result returned from a function that reads and writes.

   This is like #ExessResult but includes separate read and write counts.  This
   allows the caller to know both how many bytes were read from the input and
   how many bytes were written to the output.
*/
typedef struct {
  ExessStatus status;      ///< Status code
  size_t      read_count;  ///< Number of bytes read
  size_t      write_count; ///< Number of bytes written, excluding null
} ExessVariableResult;

/**
   Return a string describing a status code in plain language.

   The returned string is always one sentence, with an uppercase first
   character, and no trailing period.
*/
EXESS_CONST_API const char* EXESS_NONNULL
exess_strerror(ExessStatus status);

/**
   @}
   @defgroup exess_fixed Fixed Type API
   Statically type-safe API for working with values of a known fixed type.
   @{
   @defgroup exess_numbers Numbers
   @{
*/

/**
   @defgroup exess_decimal decimal

   A `decimal` is a decimal number of arbitrary precision, but this
   implementation only supports values that fit in a `double`.

   Unlike `double`, `decimal` is written in numeric form, never in scientific
   notation.  Special infinity and NaN values aren't supported.  Note that the
   `decimal` representation for some numbers is very long, so `double` may be a
   better choice for values in a wide range.

   Canonical form has no leading "+" sign, and at most 1 leading or trailing
   zero such that there is at least 1 digit on either side of the decimal
   point, like "12.34", "-1.0", and "0.0".

   Non-canonical form allows a leading "+", any number of leading and trailing
   zeros, any number of digits (including zero) on either side of the point,
   and doesn't require a decimal point, like "+1", "01", "-.5", "4.", and "42".

   @{
*/

/// The maximum length of a `decimal` string from exess_write_decimal()
#define EXESS_MAX_DECIMAL_LENGTH 327U

/**
   Read a `decimal` string after any leading whitespace.

   Values too large (in absolute terms) to store will produce infinity, values
   too small will produce zero, and insignificant digits may be discarded.
   Note that this is a limitation of this implementation and `decimal` itself
   has unlimited precision, so this may result in data loss.

   @param str String to read.
   @param[out] out Parsed value, or NaN on error.
   @return The `count` of characters read, and a `status`.
*/
EXESS_API ExessResult
exess_read_decimal(const char* EXESS_NONNULL str, double* EXESS_NONNULL out);

/**
   Write a canonical `decimal` string.

   @param value Value to write.
   @param buf_size Size of `buf` in bytes.
   @param[out] buf Output buffer, or null to only measure.

   @return The `count` of characters in the output, and a `status`
   (#EXESS_SUCCESS, or #EXESS_NO_SPACE if the buffer is too small).
*/
EXESS_API ExessResult
exess_write_decimal(double value, size_t buf_size, char* EXESS_UNSPECIFIED buf);

/**
   @}
   @defgroup exess_double double

   A `double` is an IEEE-754 64-bit floating point number, written in
   scientific notation.

   Canonical form has no leading "+" sign, at most 1 leading or trailing zero
   such that there is at least 1 digit on either side of the decimal point, and
   always has an exponent, like "12.34E56", "-1.0E-2", and "-0.0E0".  The
   special values negative infinity, positive infinity, and not-a-number are
   written "-INF", "INF", and "NaN", respectively.

   Non-canonical form allows a leading "+", any number of leading and trailing
   zeros, any number of digits (including zero) on either side of the point,
   and doesn't require an exponent or decimal point, like "+1E3", "1E+3",
   ".5E3", "4.2", and "42".

   @{
*/

/// The maximum length of a canonical `double` string
#define EXESS_MAX_DOUBLE_LENGTH 24U

/**
   Read a `double` string after any leading whitespace.

   Values too large (in absolute terms) to store will produce infinity, values
   too small will produce zero, and insignificant digits may be discarded.
   Note that these cases are considered successful since the value space of
   `double` has limited precision by definition.

   @param str String to read.
   @param[out] out Parsed value, or NaN on error.
   @return The `count` of characters read, and a `status`.
*/
EXESS_API ExessResult
exess_read_double(const char* EXESS_NONNULL str, double* EXESS_NONNULL out);

/**
   Write a canonical `double` string.

   Any `double` value is supported.  Reading the resulting string with
   exess_read_double() will produce exactly `value`, except the extra bits in
   NaNs aren't preserved.

   @param value Value to write.
   @param buf_size Size of `buf` in bytes.
   @param[out] buf Output buffer, or null to only measure.

   @return The `count` of characters in the output, and a `status`
   (#EXESS_SUCCESS, or #EXESS_NO_SPACE if the buffer is too small).
*/
EXESS_API ExessResult
exess_write_double(double value, size_t buf_size, char* EXESS_UNSPECIFIED buf);

/**
   @}
   @defgroup exess_float float

   A `float` is an IEEE-754 32-bit floating point number, written in scientific
   notation.

   The lexical form is the same as @ref exess_double, but the value space has
   less precision.

   @{
*/

/// The maximum length of a canonical `float` string
#define EXESS_MAX_FLOAT_LENGTH 15U

/**
   Read a `float` string after any leading whitespace.

   Values too large (in absolute terms) to store will produce infinity, values
   too small will produce zero, and insignificant digits may be discarded.
   Note that these cases are considered successful since the value space of
   `float` has limited precision by definition.

   @param str String to read.
   @param[out] out Parsed value, or NaN on error.
   @return The `count` of characters read, and a `status`.
*/
EXESS_API ExessResult
exess_read_float(const char* EXESS_NONNULL str, float* EXESS_NONNULL out);

/**
   Write a canonical `float` string.

   Any `float` value is supported.  Reading the resulting string with
   exess_read_float() will produce exactly `value`, except the extra bits in
   NaNs aren't preserved.

   @param value Value to write.
   @param buf_size Size of `buf` in bytes.
   @param[out] buf Output buffer, or null to only measure.

   @return The `count` of characters in the output, and a `status`
   (#EXESS_SUCCESS, or #EXESS_NO_SPACE if the buffer is too small).
*/
EXESS_API ExessResult
exess_write_float(float value, size_t buf_size, char* EXESS_UNSPECIFIED buf);

/**
   @}
   @defgroup exess_boolean boolean

   A `boolean` has only two possible values, canonically written as "false"
   and "true".  The non-canonical forms "0" and "1" are also supported.

   @{
*/

/// The maximum length of a canonical `boolean` string
#define EXESS_MAX_BOOLEAN_LENGTH 5U

/**
   Read a `boolean` string after any leading whitespace.

   @param str String to read.
   @param[out] out Parsed value, or false on error.
   @return The `count` of characters read, and a `status`.
*/
EXESS_API ExessResult
exess_read_boolean(const char* EXESS_NONNULL str, bool* EXESS_NONNULL out);

/**
   Write a canonical `boolean` string.

   @param value Value to write.
   @param buf_size Size of `buf` in bytes.
   @param[out] buf Output buffer, or null to only measure.

   @return The `count` of characters in the output, and a `status`
   (#EXESS_SUCCESS, or #EXESS_NO_SPACE if the buffer is too small).
*/
EXESS_API ExessResult
exess_write_boolean(bool value, size_t buf_size, char* EXESS_UNSPECIFIED buf);

/**
   @}
   @defgroup exess_long long

   A `long` is a signed 64-bit integer, written in decimal.

   Values range from -9223372036854775808 to 9223372036854775807 inclusive.

   Canonical form has no leading "+" sign and no leading zeros (except for the
   number "0"), like "-1", "0", and "1234".

   Non-canonical form allows a leading "+" and any number of leading zeros,
   like "01" and "+0001234".

   @{
*/

/// The maximum length of a canonical `long` string
#define EXESS_MAX_LONG_LENGTH 20U

/**
   Read a `long` string after any leading whitespace.

   @param str String to read.
   @param[out] out Parsed value, or zero on error.
   @return The `count` of characters read, and a `status`.
*/
EXESS_API ExessResult
exess_read_long(const char* EXESS_NONNULL str, int64_t* EXESS_NONNULL out);

/**
   Write a canonical `long` string.

   @param value Value to write.
   @param buf_size Size of `buf` in bytes.
   @param[out] buf Output buffer, or null to only measure.

   @return The `count` of characters in the output, and a `status`
   (#EXESS_SUCCESS, or #EXESS_NO_SPACE if the buffer is too small).
*/
EXESS_API ExessResult
exess_write_long(int64_t value, size_t buf_size, char* EXESS_UNSPECIFIED buf);

/**
   @}
   @defgroup exess_int int

   An `int` is a signed 32-bit integer.

   The lexical form is the same as @ref exess_long, but values range from
   -2147483648 to 2147483647 inclusive.

   @{
*/

/// The maximum length of a canonical `int` string
#define EXESS_MAX_INT_LENGTH 11U

/**
   Read an `int` string after any leading whitespace.

   @param str String to read.
   @param[out] out Parsed value, or zero on error.
   @return The `count` of characters read, and a `status`.
*/
EXESS_API ExessResult
exess_read_int(const char* EXESS_NONNULL str, int32_t* EXESS_NONNULL out);

/**
   Write a canonical `int` string.

   @param value Value to write.
   @param buf_size Size of `buf` in bytes.
   @param[out] buf Output buffer, or null to only measure.

   @return The `count` of characters in the output, and a `status`
   (#EXESS_SUCCESS, or #EXESS_NO_SPACE if the buffer is too small).
*/
EXESS_API ExessResult
exess_write_int(int32_t value, size_t buf_size, char* EXESS_UNSPECIFIED buf);

/**
   @}
   @defgroup exess_short short

   A `short` is a signed 16-bit integer.

   The lexical form is the same as @ref exess_long, but values range from -32768
   to 32767 inclusive.

   @{
*/

/// The maximum length of a canonical `short` string
#define EXESS_MAX_SHORT_LENGTH 6U

/**
   Read a `short` string after any leading whitespace.

   @param str String to read.
   @param[out] out Parsed value, or zero on error.
   @return The `count` of characters read, and a `status`.
*/
EXESS_API ExessResult
exess_read_short(const char* EXESS_NONNULL str, int16_t* EXESS_NONNULL out);

/**
   Write a canonical `short` string.

   @param value Value to write.
   @param buf_size Size of `buf` in bytes.
   @param[out] buf Output buffer, or null to only measure.

   @return The `count` of characters in the output, and a `status`
   (#EXESS_SUCCESS, or #EXESS_NO_SPACE if the buffer is too small).
*/
EXESS_API ExessResult
exess_write_short(int16_t value, size_t buf_size, char* EXESS_UNSPECIFIED buf);

/**
   @}
   @defgroup exess_byte byte

   A `byte` is a signed 8-bit integer.

   The lexical form is the same as `long`, but values range from -128 to 127
   inclusive.

   @{
*/

/// The maximum length of a canonical `byte` string
#define EXESS_MAX_BYTE_LENGTH 4U

/**
   Read a `byte` string after any leading whitespace.

   @param str String to read.
   @param[out] out Parsed value, or zero on error.
   @return The `count` of characters read, and a `status`.
*/
EXESS_API ExessResult
exess_read_byte(const char* EXESS_NONNULL str, int8_t* EXESS_NONNULL out);

/**
   Write a canonical `byte` string.

   @param value Value to write.
   @param buf_size Size of `buf` in bytes.
   @param[out] buf Output buffer, or null to only measure.

   @return The `count` of characters in the output, and a `status`
   (#EXESS_SUCCESS, or #EXESS_NO_SPACE if the buffer is too small).
*/
EXESS_API ExessResult
exess_write_byte(int8_t value, size_t buf_size, char* EXESS_UNSPECIFIED buf);

/**
   @}
   @defgroup exess_ulong unsignedLong

   An `unsignedLong` is an unsigned 64-bit integer, written in decimal.

   Values range from 0 to 18446744073709551615 inclusive.

   Canonical form has no leading "+" sign and no leading zeros (except for the
   number "0"), like "0", and "1234".

   Non-canonical form allows a leading "+", a leading "-" for zero, and
   any number of leading zeros, like "01" and "0001234".

   @{
*/

/// The maximum length of a canonical `unsignedLong` string
#define EXESS_MAX_ULONG_LENGTH 20U

/**
   Read an `unsignedLong` string after any leading whitespace.

   @param str String to read.
   @param[out] out Parsed value, or zero on error.
   @return The `count` of characters read, and a `status`.
*/
EXESS_API ExessResult
exess_read_ulong(const char* EXESS_NONNULL str, uint64_t* EXESS_NONNULL out);

/**
   Write a canonical `unsignedLong` string.

   @param value Value to write.
   @param buf_size Size of `buf` in bytes.
   @param[out] buf Output buffer, or null to only measure.

   @return The `count` of characters in the output, and a `status`
   (#EXESS_SUCCESS, or #EXESS_NO_SPACE if the buffer is too small).
*/
EXESS_API ExessResult
exess_write_ulong(uint64_t value, size_t buf_size, char* EXESS_UNSPECIFIED buf);

/**
   @}
   @defgroup exess_uint unsignedInt

   An `unsignedInt` is an unsigned 32-bit integer.

   The lexical form is the same as @ref exess_ulong, but values range from 0 to
   4294967295 inclusive.

   @{
*/

/// The maximum length of a canonical `unsignedInt` string
#define EXESS_MAX_UINT_LENGTH 10U

/**
   Read an `unsignedInt` string after any leading whitespace.

   @param str String to read.
   @param[out] out Parsed value, or zero on error.
   @return The `count` of characters read, and a `status`.
*/
EXESS_API ExessResult
exess_read_uint(const char* EXESS_NONNULL str, uint32_t* EXESS_NONNULL out);

/**
   Write a canonical `unsignedInt` string.

   @param value Value to write.
   @param buf_size Size of `buf` in bytes.
   @param[out] buf Output buffer, or null to only measure.

   @return The `count` of characters in the output, and a `status`
   (#EXESS_SUCCESS, or #EXESS_NO_SPACE if the buffer is too small).
*/
EXESS_API ExessResult
exess_write_uint(uint32_t value, size_t buf_size, char* EXESS_UNSPECIFIED buf);

/**
   @}
   @defgroup exess_ushort unsignedShort

   An `unsignedShort` is an unsigned 16-bit integer.

   The lexical form is the same as @ref exess_ulong, but values range from 0 to
   65535 inclusive.

   @{
*/

/// The maximum length of a canonical `unsignedShort` string
#define EXESS_MAX_USHORT_LENGTH 5U

/**
   Read an `unsignedShort` string after any leading whitespace.

   @param str String to read.
   @param[out] out Parsed value, or zero on error.
   @return The `count` of characters read, and a `status`.
*/
EXESS_API ExessResult
exess_read_ushort(const char* EXESS_NONNULL str, uint16_t* EXESS_NONNULL out);

/**
   Write a canonical `unsignedShort` string.

   @param value Value to write.
   @param buf_size Size of `buf` in bytes.
   @param[out] buf Output buffer, or null to only measure.

   @return The `count` of characters in the output, and a `status`
   (#EXESS_SUCCESS, or #EXESS_NO_SPACE if the buffer is too small).
*/
EXESS_API ExessResult
exess_write_ushort(uint16_t                value,
                   size_t                  buf_size,
                   char* EXESS_UNSPECIFIED buf);

/**
   @}
   @defgroup exess_ubyte unsignedByte

   An `unsignedByte` is an unsigned 8-bit integer.

   The lexical form is the same as @ref exess_ulong, but values range from 0 to
   255 inclusive.

   @{
*/

/// The maximum length of a canonical `unsignedByte` string
#define EXESS_MAX_UBYTE_LENGTH 3U

/**
   Read an `unsignedByte` string after any leading whitespace.

   @param str String to read.
   @param[out] out Parsed value, or zero on error.
   @return The `count` of characters read, and a `status`.
*/
EXESS_API ExessResult
exess_read_ubyte(const char* EXESS_NONNULL str, uint8_t* EXESS_NONNULL out);

/**
   Write a canonical `unsignedByte` string.

   @param value Value to write.
   @param buf_size Size of `buf` in bytes.
   @param[out] buf Output buffer, or null to only measure.

   @return The `count` of characters in the output, and a `status`
   (#EXESS_SUCCESS, or #EXESS_NO_SPACE if the buffer is too small).
*/
EXESS_API ExessResult
exess_write_ubyte(uint8_t value, size_t buf_size, char* EXESS_UNSPECIFIED buf);

/**
   @}
   @}
   @defgroup exess_time_and_date Time and Date
   @{
*/

/**
   @defgroup exess_timezone Timezone Offsets

   Some time and date values can have a timezone qualifier suffix.  A timezone
   isn't a datatype unto itself, but only exists as a part of another value.

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

/// Sentinel value for local time
#define EXESS_TIMEZONE_LOCAL ((ExessTimezone)127U)

/// Sentinel value for UTC time
#define EXESS_TIMEZONE_UTC ((ExessTimezone)0U)

/**
   Construct a time zone offset from hours and minutes.

   This is a convenience constructor that handles the conversion from hours and
   minutes to the quarter-hour offset used in exess.  The sign of both values
   must be the same.  Hours can be from -14 to 14 inclusive, and minutes can
   only be -45, -30, -15, 0, 15, 30, or 45.

   @return A time zone offset in quarter hours, or #EXESS_TIMEZONE_LOCAL if the
   parameters are invalid or not supported.
*/
EXESS_CONST_API ExessTimezone
exess_timezone(int8_t hours, int8_t minutes);

/**
   @}
   @defgroup exess_duration duration

   A `duration` is a positive or negative duration of time, written in ISO
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

/// The maximum length of a `duration` string from exess_write_duration()
#define EXESS_MAX_DURATION_LENGTH 41U

/**
   Duration of time.

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

   Loosely speaking, a duration is considered less than another if it's a
   shorter duration of time.  However, note that two durations may not be
   comparable, since the relation between fields, such as the number of days in
   a month, varies at different times.  Strictly speaking, two durations are
   comparable if adding them to any `dateTime` would produce results with the
   same ordering.

   The #ExessDuration representation condenses all fields into two values:
   months and seconds.  When the two values are incomparable, a month is
   considered greater than any number of seconds.

   Values with only months, or only seconds, are always comparable.
   Applications are encouraged to always use such values and never carry
   seconds into months or vice-versa.

   @return Less than, equal to, or greater than zero if `lhs` is less than,
   equal to, or greater than `rhs`, respectively.  Comparable and incomparable
   cases may also be distinguished, see #ExessOrder for details.
*/
EXESS_CONST_API ExessOrder
exess_compare_duration(ExessDuration lhs, ExessDuration rhs);

/**
   Read a `duration` string after any leading whitespace.

   @param str String to read.
   @param[out] out Parsed value, or zero on error.
   @return The `count` of characters read, and a `status`.
*/
EXESS_API ExessResult
exess_read_duration(const char* EXESS_NONNULL    str,
                    ExessDuration* EXESS_NONNULL out);

/**
   Write a canonical `duration` string.

   @param value Value to write.
   @param buf_size Size of `buf` in bytes.
   @param[out] buf Output buffer, or null to only measure.

   @return #EXESS_SUCCESS on success, #EXESS_NO_SPACE if the buffer is too
   small, or #EXESS_BAD_VALUE if the value is invalid.
*/
EXESS_API ExessResult
exess_write_duration(ExessDuration           value,
                     size_t                  buf_size,
                     char* EXESS_UNSPECIFIED buf);
/**
   @}
   @defgroup exess_date_time dateTime

   A `dateTime` is a date and time, with an optional timezone offset.

   Strings have the form YYYY-MM-DDTHH:MM:SS with an optional timezone suffix,
   at least 4 year digits (negative or positive), and all other fields positive
   two-digit integers except seconds which may be a decimal.  For example,
   "2001-02-03T12:13:14.56" or "2001-02-03T12:13:14.56-06:00"

   Canonical form only includes a decimal point if the number of seconds isn't
   an integer.  This implementation supports up to nanosecond resolution.
   Midnight at the end of the day, like "1999-12-31T24:00:00" is a valid value,
   equivalent to the start of the next day, like "2000-01-01T00:00:00", which
   is the canonical form.

   @{
*/

/// The maximum length of a `dateTime` string from exess_write_date_time()
#define EXESS_MAX_DATE_TIME_LENGTH 37U

/**
   Date and time.

   This representation follows the syntax, except the timezone offset is stored
   between the date and time for more efficient packing.
*/
typedef struct {
  int16_t       year;       ///< Year: any positive or negative value
  uint8_t       month;      ///< Month: [1, 12]
  uint8_t       day;        ///< Day: [1, 31]
  ExessTimezone zone;       ///< Timezone offset in quarter hours
  uint8_t       hour;       ///< Hour: [0, 24]
  uint8_t       minute;     ///< Minute: [0, 59]
  uint8_t       second;     ///< Second: [0, 59]
  uint32_t      nanosecond; ///< Nanosecond: [0, 999999999]
} ExessDateTime;

/**
   Compare two dateTimes.

   Note that local and zoned dateTimes times may not be comparable.  When the
   values aren't comparable, this function arbitrarily chooses the local time
   to be lesser.

   @return Less than, equal to, or greater than zero if `lhs` is less than,
   equal to, or greater than `rhs`, respectively.  Comparable and incomparable
   cases may also be distinguished, see #ExessOrder for details.
*/
EXESS_CONST_API ExessOrder
exess_compare_date_time(ExessDateTime lhs, ExessDateTime rhs);

/**
   Add a duration to a dateTime.

   This advances or rewinds by the given duration, depending on whether the
   duration is positive or negative.

   If underflow or overflow occur, then this will return an infinite value.  A
   positive infinity has all fields at maximum, and a negative infinity has all
   fields at minimum, except `zone` which is preserved from the input (so
   infinities are comparable with the values they came from).  Since 0 and 255
   are never valid months, these can be tested for by checking if the year and
   month are `INT16_MIN` and 0, or `INT16_MAX` and `INT8_MAX`.

   @return `s + d`, or an infinite past or infinite future if underflow or
   overflow occurs.
*/
EXESS_CONST_API ExessDateTime
exess_add_date_time_duration(ExessDateTime s, ExessDuration d);

/**
   Convert a dateTime to UTC.

   @return The input converted to UTC if it has a timezone, otherwise the
   unmodified input.
*/
EXESS_CONST_API ExessDateTime
exess_date_time_to_utc(ExessDateTime datetime);

/**
   Read a dateTime string after any leading whitespace.

   @param str String to read.
   @param[out] out Parsed value, or zero on error.
   @return The `count` of characters read, and a `status`.
*/
EXESS_API ExessResult
exess_read_date_time(const char* EXESS_NONNULL    str,
                     ExessDateTime* EXESS_NONNULL out);

/**
   Write a dateTime string.

   The written string is in canonical form, except that midnight at the end of
   the day (24:00:00) is written as-is.  Note that this differs from the other
   write functions, which always write canonical form.

   @param value Value to write.
   @param buf_size Size of `buf` in bytes.
   @param[out] buf Output buffer, or null to only measure.

   @return #EXESS_SUCCESS on success, #EXESS_NO_SPACE if the buffer is too
   small, or #EXESS_BAD_VALUE if the value is invalid.
*/
EXESS_API ExessResult
exess_write_date_time(ExessDateTime           value,
                      size_t                  buf_size,
                      char* EXESS_UNSPECIFIED buf);

/**
   @}
   @defgroup exess_date date

   A `date` is a year, month, and day, with an optional timezone offset.

   Strings have the form YYYY-MM-DD with an optional timezone suffix, at least
   4 year digits (negative or positive), and exactly 2 digits for both month
   and day.  For example, "2001-02-03" or "2001-02-03-06:00".

   Canonical form has no leading zeros for the year.

   @{
*/

/// The maximum length of a `date` string from exess_write_date()
#define EXESS_MAX_DATE_LENGTH 18U

/// Date
typedef struct {
  int16_t       year;  ///< Year
  uint8_t       month; ///< Month: [1, 12]
  uint8_t       day;   ///< Day: [1, 31]
  ExessTimezone zone;  ///< Timezone offset in quarter hours
} ExessDate;

/**
   Compare two dates.

   Note that local and zoned dates may not be comparable.  See
   exess_compare_date_time() for details.

   @return Less than, equal to, or greater than zero if `lhs` is less than,
   equal to, or greater than `rhs`, respectively.  Comparable and incomparable
   cases may also be distinguished, see #ExessOrder for details.
*/
EXESS_CONST_API ExessOrder
exess_compare_date(ExessDate lhs, ExessDate rhs);

/**
   Read a `date` string after any leading whitespace.

   @param str String to read.
   @param[out] out Parsed value, or zero on error.
   @return The `count` of characters read, and a `status`.
*/
EXESS_API ExessResult
exess_read_date(const char* EXESS_NONNULL str, ExessDate* EXESS_NONNULL out);

/**
   Write a canonical `date` string.

   The output is always in canonical form, like `2001-04-12` or
   `-2001-10-26+02:00`.

   @param value Value to write.
   @param buf_size Size of `buf` in bytes.
   @param[out] buf Output buffer, or null to only measure.

   @return #EXESS_SUCCESS on success, #EXESS_NO_SPACE if the buffer is too
   small, or #EXESS_BAD_VALUE if the value is invalid.
*/
EXESS_API ExessResult
exess_write_date(ExessDate value, size_t buf_size, char* EXESS_UNSPECIFIED buf);

/**
   @}
   @defgroup exess_time time

   A `time` is a time of day, with an optional timezone offset.

   Strings have the form HH:MM:SS with an optional timezone suffix, where
   seconds may be a decimal value, for example, "12:13:14", "12:13:14.56", or
   "12:13:14.56-07:00".

   Canonical form only includes a decimal point if the number of seconds isn't
   an integer.  This implementation supports up to nanosecond resolution.  Note
   that midnight at the end of the day (like "24:00:00") is a valid lexical
   form, but unlike `dateTime`, 24 isn't a valid hour value.  Such forms will
   be read as 00:00:00.

   @{
*/

/// The maximum length of a `time` string from exess_write_time()
#define EXESS_MAX_TIME_LENGTH 24U

/// Time
typedef struct {
  ExessTimezone zone;       ///< Timezone offset in quarter hours
  uint8_t       hour;       ///< Hour: [0, 23]
  uint8_t       minute;     ///< Minute: [0, 59]
  uint8_t       second;     ///< Second: [0, 59]
  uint32_t      nanosecond; ///< Nanosecond: [0, 999999999]
} ExessTime;

/**
   Compare two times.

   Note that local and zoned times may not be comparable.  See
   exess_compare_date_time() for details.

   @return Less than, equal to, or greater than zero if `lhs` is less than,
   equal to, or greater than `rhs`, respectively.  Comparable and incomparable
   cases may also be distinguished, see #ExessOrder for details.
*/
EXESS_CONST_API ExessOrder
exess_compare_time(ExessTime lhs, ExessTime rhs);

/**
   Read a `time` string after any leading whitespace.

   @param str String to read.
   @param[out] out Parsed value, or zero on error.
   @return The `count` of characters read, and a `status`.
*/
EXESS_API ExessResult
exess_read_time(const char* EXESS_NONNULL str, ExessTime* EXESS_NONNULL out);

/**
   Write a canonical `time` string.

   The output is always in canonical form, like "12:15" or "02:00Z".

   @param value Value to write.
   @param buf_size Size of `buf` in bytes.
   @param[out] buf Output buffer, or null to only measure.

   @return #EXESS_SUCCESS on success, #EXESS_NO_SPACE if the buffer is too
   small, or #EXESS_BAD_VALUE if the value is invalid.
*/
EXESS_API ExessResult
exess_write_time(ExessTime value, size_t buf_size, char* EXESS_UNSPECIFIED buf);

/**
   @}
   @}
   @defgroup exess_binary Binary
   @{
*/

/**
   @defgroup exess_base64 base64Binary

   A `base64Binary` is arbitrary binary data in base64 encoding.

   Strings consist of characters from the base64 alphabet ([0-9], [A-Z], [a-z],
   "+", "/", and "=") with whitespace (tab, newline, carriage return, or space)
   allowed anywhere.  Strings are padded to a multiple of four non-whitespace
   characters with with trailing "=" characters.  For example, the base64
   encoding of the string "data" is "ZGF0YQo=".

   Canonical form contains no whitespace.

   @{
*/

/**
   Return the maximum number of bytes required to decode `length` bytes of
   base64.

   The returned value is an upper bound which is only exact for canonical
   strings.

   @param length Number of input (text) bytes to decode.
   @return The maximum size of a decoded value in bytes.
*/
EXESS_CONST_API size_t
exess_decoded_base64_size(size_t length);

/**
   Read a binary value from a base64 string.

   Canonical syntax is a multiple of 4 base64 characters, with either 1 or 2
   trailing "=" characters as necessary, like "Zm9vYg==", with no whitespace.
   All whitespace is skipped when reading.

   The caller must allocate a large enough buffer to read the value, otherwise
   an #EXESS_NO_SPACE error will be returned.  The required space can be
   calculated with exess_decoded_base64_size().

   When this is called, `out` must point to a buffer of at least `out_size`
   bytes.  The returned result contains the exact size of the decoded data,
   which may be smaller than `out_size`.  Only these first bytes are written,
   the rest of the buffer isn't modified.

   @param str String to read.
   @param out_size Size of `out` in bytes.
   @param[out] out Decoded binary data.

   @return The `read_count` of characters read, `write_count` of bytes written,
   and a `status`.
*/
EXESS_API ExessVariableResult
exess_read_base64(const char* EXESS_NONNULL str,
                  size_t                    out_size,
                  void* EXESS_NONNULL       out);

/**
   Write a canonical `base64Binary` string.

   The data is always written in canonical form, as a multiple of 4 characters
   with no whitespace and 1 or 2 trailing "=" characters as padding if
   necessary.

   @param data_size Size of `data` in bytes.
   @param data Data to write.
   @param buf_size Size of `buf` in bytes.
   @param[out] buf Output buffer, or null to only measure.

   @return The `count` of characters in the output, and a `status`
   (#EXESS_SUCCESS, or #EXESS_NO_SPACE if the buffer is too small).
*/
EXESS_API ExessResult
exess_write_base64(size_t                    data_size,
                   const void* EXESS_NONNULL data,
                   size_t                    buf_size,
                   char* EXESS_UNSPECIFIED   buf);

/**
   @}
   @defgroup exess_hex hexBinary

   A `hexBinary` is arbitrary binary data in hexadecimal encoding.

   Strings consist of an even number of hexadecimal characters ([0-9], [A-F],
   and [a-f]).  Note that, unlike `base64Binary`, whitespace between characters
   isn't allowed.

   @{
*/

/**
   Return the maximum number of bytes required to decode `length` bytes of hex.

   The returned value is an upper bound which is only exact for canonical
   strings.

   @param length Number of input (text) bytes to decode.
   @return The size of a decoded value in bytes.
*/
EXESS_CONST_API size_t
exess_decoded_hex_size(size_t length);

/**
   Read a binary value from a hex string.

   Canonical syntax is an even number of uppercase hexadecimal digits with no
   whitespace, like "666F6F".  Lowercase hexadecimal is also supported, and all
   whitespace is skipped when reading.

   The caller must allocate a large enough buffer to read the value, otherwise
   an #EXESS_NO_SPACE error will be returned.  The required space can be
   calculated with exess_decoded_hex_size().

   When this is called, `out` must point to a buffer of at least `out_size`
   bytes.  The returned result contains the exact size of the decoded data,
   which may be smaller than `out_size`.  Only these first bytes are written,
   the rest of the buffer isn't modified.

   @param str String to parse.
   @param out_size Size of `out` in bytes.
   @param[out] out Decoded binary data.

   @return The `read_count` of characters read, `write_count` of bytes written,
   and a `status`.
*/
EXESS_API ExessVariableResult
exess_read_hex(const char* EXESS_NONNULL str,
               size_t                    out_size,
               void* EXESS_NONNULL       out);

/**
   Write a canonical `hexBinary` string.

   The data is always written in canonical form, as an even number of uppercase
   hexadecimal digits with no whitespace.

   @param data_size Size of `data` in bytes.
   @param data Data to write.
   @param buf_size Size of `buf` in bytes.
   @param[out] buf Output buffer, or null to only measure.

   @return The `count` of characters in the output, and a `status`
   (#EXESS_SUCCESS, or #EXESS_NO_SPACE if the buffer is too small).
*/
EXESS_API ExessResult
exess_write_hex(size_t                    data_size,
                const void* EXESS_NONNULL data,
                size_t                    buf_size,
                char* EXESS_UNSPECIFIED   buf);

/**
   @}
   @}
   @}
   @defgroup exess_dynamic Dynamic Type API
   API for working with general values of a type given at runtime.
   @{
   @defgroup exess_datatypes Datatypes

   Integer identifiers for datatypes, and utility functions to access their
   properties.

   @{
*/

/// A supported datatype, or zero
typedef enum {
  EXESS_NOTHING,              ///< Sentinel for unknown datatypes or errors
  EXESS_BOOLEAN,              ///< @ref exess_boolean
  EXESS_DECIMAL,              ///< @ref exess_decimal
  EXESS_DOUBLE,               ///< @ref exess_double
  EXESS_FLOAT,                ///< @ref exess_float
  EXESS_INTEGER,              ///< `integer` as @ref exess_long
  EXESS_NON_POSITIVE_INTEGER, ///< `nonPositiveInteger` as @ref exess_long
  EXESS_NEGATIVE_INTEGER,     ///< `negativeInteger` as @ref exess_long
  EXESS_LONG,                 ///< @ref exess_long
  EXESS_INT,                  ///< @ref exess_int
  EXESS_SHORT,                ///< @ref exess_short
  EXESS_BYTE,                 ///< @ref exess_byte
  EXESS_NON_NEGATIVE_INTEGER, ///< `nonNegativeInteger` as @ref exess_ulong
  EXESS_ULONG,                ///< @ref exess_ulong
  EXESS_UINT,                 ///< @ref exess_uint
  EXESS_USHORT,               ///< @ref exess_ushort
  EXESS_UBYTE,                ///< @ref exess_ubyte
  EXESS_POSITIVE_INTEGER,     ///< `positiveInteger` as @ref exess_ulong
  EXESS_DURATION,             ///< @ref exess_duration
  EXESS_DATE_TIME,            ///< @ref exess_date_time
  EXESS_TIME,                 ///< @ref exess_time
  EXESS_DATE,                 ///< @ref exess_date
  EXESS_HEX,                  ///< @ref exess_hex
  EXESS_BASE64,               ///< @ref exess_base64
} ExessDatatype;

/// The unsigned value of the largest #ExessDatatype enumerator
#define EXESS_MAX_DATATYPE 23U

/**
   Array of maximum string lengths, indexed by datatype.

   Each entry is the maximum length of a string with the indexed datatype, or
   zero if the datatype is unknown or unbounded.  For example:

   @code
   unsigned four_bytes = exess_max_lengths[EXESS_BYTE];
   @endcode

   The unbounded types are #EXESS_DECIMAL, #EXESS_INTEGER and its half-bounded
   subtypes #EXESS_NON_POSITIVE_INTEGER, #EXESS_NEGATIVE_INTEGER,
   #EXESS_NON_NEGATIVE_INTEGER, and #EXESS_POSITIVE_INTEGER, and the binary
   types #EXESS_HEX and #EXESS_BASE64.
*/
static EXESS_CONSTEXPR uint8_t exess_max_lengths[EXESS_MAX_DATATYPE + 1U] = {
  0U,  // Unknown datatype
  5U,  // boolean
  0U,  // decimal
  24U, // double
  15U, // float
  0U,  // integer
  0U,  // nonPositiveInteger
  0U,  // negativeInteger
  20U, // long
  11U, // int
  6U,  // short
  4U,  // byte
  0U,  // nonNegativeInteger
  20U, // ulong
  10U, // uint
  5U,  // ushort
  3U,  // ubyte
  0U,  // positiveInteger
  41U, // duration
  32U, // dateTime
  24U, // time
  18U, // date
  0U,  // hexBinary
  0U,  // base64Binary
};

/**
   Array of binary value sizes, indexed by datatype.

   Each entry is the size of the binary representation of the indexed datatype
   (not a string length), or zero for #EXESS_HEX and #EXESS_BASE64, since such
   values can be arbitrarily large.
*/
static EXESS_CONSTEXPR uint8_t exess_value_sizes[EXESS_MAX_DATATYPE + 1U] = {
  0U,                    // Unknown
  sizeof(bool),          // boolean
  sizeof(double),        // decimal
  sizeof(double),        // double
  sizeof(float),         // float
  sizeof(int64_t),       // integer
  sizeof(int64_t),       // nonPositiveInteger
  sizeof(int64_t),       // negativeInteger
  sizeof(int64_t),       // long
  sizeof(int32_t),       // int
  sizeof(int16_t),       // short
  sizeof(int8_t),        // byte
  sizeof(uint64_t),      // nonNegativeInteger
  sizeof(uint64_t),      // ulong
  sizeof(uint32_t),      // uint
  sizeof(uint16_t),      // ushort
  sizeof(uint8_t),       // ubyte
  sizeof(uint64_t),      // positiveInteger
  sizeof(ExessDuration), // duration
  sizeof(ExessDateTime), // dateTime
  sizeof(ExessTime),     // time
  sizeof(ExessDate),     // date
  0U,                    // hexBinary
  0U,                    // base64Binary
};

/**
   Return the URI for a datatype, or null.

   This only returns URIs that start with "http://www.w3.org/2001/XMLSchema#".

   @param datatype Datatype tag.
   @return The URI of the datatype, or null for #EXESS_NOTHING.
*/
EXESS_CONST_API const char* EXESS_NULLABLE
exess_datatype_uri(ExessDatatype datatype);

/**
   Return the name of a datatype, or null.

   This returns the suffix of the URI that would be returned by
   exess_datatype_uri().

   @param datatype Datatype tag.
   @return The name of the datatype, or null for #EXESS_NOTHING.
*/
EXESS_CONST_API const char* EXESS_NULLABLE
exess_datatype_name(ExessDatatype datatype);

/**
   Return the datatype tag for a datatype URI.

   @param uri Absolute URI for a supported datatype, like
   "http://www.w3.org/2001/XMLSchema#unsignedLong" or
   "http://www.w3.org/2001/XMLSchema#dateTime".

   @return A datatype tag, or #EXESS_NOTHING if no such datatype is known.
*/
EXESS_PURE_API ExessDatatype
exess_datatype_from_uri(const char* EXESS_NONNULL uri);

/**
   Return the datatype tag for a datatype name.

   @param name The camel-case name of a supported datatype (its URI fragment),
   like "unsignedLong" or "dateTime".

   @return A datatype tag, or #EXESS_NOTHING if no such datatype is known.
*/
EXESS_PURE_API ExessDatatype
exess_datatype_from_name(const char* EXESS_NONNULL name);

/**
   @}
   @defgroup exess_value Generic Values
   A generic interface for reading and writing binary values.
   @{
*/

/// The maximum size of a supported fixed-size value in bytes
#define EXESS_MAX_FIXED_SIZE 12U

/// Attribute to align a buffer for any supported value
#if (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L)
#  define EXESS_ALIGN _Alignas(16)
#elif (defined(__cplusplus) && __cplusplus >= 201103L)
#  define EXESS_ALIGN alignas(16)
#elif defined(__GNUC__)
#  define EXESS_ALIGN __attribute__((aligned(16)))
#else
#  define EXESS_ALIGN
#endif

/**
   Compare two values.

   @return Less than, equal to, or greater than zero if the left-hand value is
   less than, equal to, or greater than the right-hand value, respectively.
   Comparable and incomparable cases may also be distinguished, see #ExessOrder
   for details.
*/
EXESS_PURE_API ExessOrder
exess_compare_value(ExessDatatype             lhs_datatype,
                    size_t                    lhs_size,
                    const void* EXESS_NONNULL lhs_value,
                    ExessDatatype             rhs_datatype,
                    size_t                    rhs_size,
                    const void* EXESS_NONNULL rhs_value);

/**
   Read any supported datatype from a string.

   Note that `out` must be suitably aligned for the datatype being read, it
   will be dereferenced directly as a pointer to the value type.  A 64-bit
   aligned buffer will be suitably aligned for any datatype.

   @param str String to read.
   @param datatype Datatype to read the string as.
   @param out_size Size of `out` in bytes.
   @param[out] out Parsed value on success.

   @return The `read_count` from `str`, `write_count` to `out` (both in bytes),
   and a `status`.
*/
EXESS_API ExessVariableResult
exess_read_value(ExessDatatype             datatype,
                 const char* EXESS_NONNULL str,
                 size_t                    out_size,
                 void* EXESS_NONNULL       out);

/**
   Write any supported datatype to a canonical string.

   Note that `value` must be suitably aligned for the datatype being written,
   it will be dereferenced directly as a pointer to the value type.  A 64-bit
   aligned buffer will be suitably aligned for any datatype.

   @param datatype Datatype of `value`.
   @param value_size Size of `value` in bytes.
   @param value Value to write.
   @param buf_size Size of `buf` in bytes.
   @param[out] buf Output buffer, or null to only measure.

   @return The `count` of characters in the output, and a `status`
   (#EXESS_SUCCESS, or #EXESS_NO_SPACE if the buffer is too small).
*/
EXESS_API ExessResult
exess_write_value(ExessDatatype             datatype,
                  size_t                    value_size,
                  const void* EXESS_NONNULL value,
                  size_t                    buf_size,
                  char* EXESS_UNSPECIFIED   buf);

/**
   @}
   @defgroup exess_canon Canonical Form
   Rewriting generic strings in canonical form.
   @{
*/

/**
   Rewrite a supported datatype in canonical form.

   @param datatype Datatype of value.
   @param str Input value string.
   @param buf_size Size of `buf` in bytes.
   @param[out] buf Output buffer, or null to only measure.

   @return The `read_count` from `str`, `write_count` to `buf` (both in bytes),
   and a `status`.  The status may be an error from reading or writing.
*/
EXESS_API ExessVariableResult
exess_write_canonical(ExessDatatype             datatype,
                      const char* EXESS_NONNULL str,
                      size_t                    buf_size,
                      char* EXESS_UNSPECIFIED   buf);

/**
   @}
   @defgroup exess_coercion Coercion
   Coercing values to different datatypes.
   @{
*/

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
  EXESS_APPROXIMATE = 1U << 0U,

  /**
     Allow coercions that round to the nearest integer.

     This allows coercing floating point numbers to integers by rounding to the
     nearest integer, with halfway cases rounding towards even (the default
     IEEE-754 rounding order).
  */
  EXESS_ROUND = 1U << 1U,

  /**
     Allow coercions that truncate significant parts of values.

     Specifically, this allows coercing any number to boolean, and dateTime to
     date or time.
  */
  EXESS_TRUNCATE = 1U << 2U,
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

/**
   Coerce a value to another datatype if possible.

   @param coercions Enabled coercion flags.  If this is zero, then
   #EXESS_SUCCESS is only returned if the resulting value can be coerced back
   to the original type without any loss of data.  Otherwise, the lossy
   coercions enabled by the set bits will be attempted.

   @param in_datatype Datatype of `in`.
   @param in_size Size of `in` in bytes.
   @param in Input value to coerce.
   @param out_datatype Datatype to convert to.
   @param out_size Size of `out` in bytes.
   @param[out] out Coerced value on success.

   @return The `count` of bytes written, and a `status`: #EXESS_SUCCESS on
   successful conversion, #EXESS_OUT_OF_RANGE if the value is outside the range
   of the target type, #EXESS_LOSS if the required coercion isn't enabled, or
   #EXESS_UNSUPPORTED if conversion between the types isn't supported at all.
*/
EXESS_API ExessResult
exess_coerce_value(ExessCoercions            coercions,
                   ExessDatatype             in_datatype,
                   size_t                    in_size,
                   const void* EXESS_NONNULL in,
                   ExessDatatype             out_datatype,
                   size_t                    out_size,
                   void* EXESS_NONNULL       out);

/**
   @}
   @}
   @}
*/

#ifdef __cplusplus
} // extern "C"
#endif

#if defined(__GNUC__) && __GNUC__ >= 7
_Pragma("GCC diagnostic pop")
#endif

#endif // EXESS_EXESS_H
