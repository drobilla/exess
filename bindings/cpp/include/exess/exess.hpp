// Copyright 2021 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#ifndef EXESS_EXESS_HPP
#define EXESS_EXESS_HPP

#include "exess/exess.h"

#include <cstdint>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>

namespace exess {

/**
   @defgroup exesspp Exess C++ API
   This is the C++ wrapper for the exess API.
   @{
*/

constexpr const char* EXESS_NONNULL const xsd_uri =
  "http://www.w3.org/2001/XMLSchema#";

using CoercionFlags = ExessCoercionFlags;
using Datatype      = ExessDatatype;
using Date          = ExessDate;
using DateTime      = ExessDateTime;
using Duration      = ExessDuration;
using Result        = ExessResult;
using Time          = ExessTime;

/**
   @defgroup exesspp_status Status
   @copydoc exess_status
   @{
*/

/// @copydoc ExessStatus
enum class Status {
  success,                ///< @copydoc EXESS_SUCCESS
  expected_end,           ///< @copydoc EXESS_EXPECTED_END
  expected_boolean,       ///< @copydoc EXESS_EXPECTED_BOOLEAN
  expected_integer,       ///< @copydoc EXESS_EXPECTED_INTEGER
  expected_duration,      ///< @copydoc EXESS_EXPECTED_DURATION
  expected_sign,          ///< @copydoc EXESS_EXPECTED_SIGN
  expected_digit,         ///< @copydoc EXESS_EXPECTED_DIGIT
  expected_colon,         ///< @copydoc EXESS_EXPECTED_COLON
  expected_dash,          ///< @copydoc EXESS_EXPECTED_DASH
  expected_time_sep,      ///< @copydoc EXESS_EXPECTED_TIME_SEP
  expected_time_tag,      ///< @copydoc EXESS_EXPECTED_TIME_TAG
  expected_date_tag,      ///< @copydoc EXESS_EXPECTED_DATE_TAG
  expected_hex,           ///< @copydoc EXESS_EXPECTED_HEX
  expected_base64,        ///< @copydoc EXESS_EXPECTED_BASE64
  bad_order,              ///< @copydoc EXESS_BAD_ORDER
  bad_value,              ///< @copydoc EXESS_BAD_VALUE
  out_of_range,           ///< @copydoc EXESS_OUT_OF_RANGE
  no_space,               ///< @copydoc EXESS_NO_SPACE
  would_reduce_precision, ///< @copydoc EXESS_WOULD_REDUCE_PRECISION
  would_round,            ///< @copydoc EXESS_WOULD_ROUND
  would_truncate,         ///< @copydoc EXESS_WOULD_TRUNCATE
  unsupported,            ///< @copydoc EXESS_UNSUPPORTED
};

/// @copydoc exess_strerror
inline const char* EXESS_NONNULL
strerror(const Status status)
{
  return exess_strerror(static_cast<ExessStatus>(status));
}

inline std::ostream&
operator<<(std::ostream& stream, const Status status)
{
  return stream << strerror(status);
}

/**
   @}
*/

namespace detail {

template<class T>
struct DatatypeTraits {};

template<>
struct DatatypeTraits<bool> {
  static constexpr auto datatype       = EXESS_BOOLEAN;
  static constexpr auto read_function  = exess_read_boolean;
  static constexpr auto write_function = exess_write_boolean;
};

template<>
struct DatatypeTraits<double> {
  static constexpr auto datatype       = EXESS_DOUBLE;
  static constexpr auto read_function  = exess_read_double;
  static constexpr auto write_function = exess_write_double;
};

template<>
struct DatatypeTraits<float> {
  static constexpr auto datatype       = EXESS_FLOAT;
  static constexpr auto read_function  = exess_read_float;
  static constexpr auto write_function = exess_write_float;
};

template<>
struct DatatypeTraits<int64_t> {
  static constexpr auto datatype       = EXESS_LONG;
  static constexpr auto read_function  = exess_read_long;
  static constexpr auto write_function = exess_write_long;
};

template<>
struct DatatypeTraits<int32_t> {
  static constexpr auto datatype       = EXESS_INT;
  static constexpr auto read_function  = exess_read_int;
  static constexpr auto write_function = exess_write_int;
};

template<>
struct DatatypeTraits<int16_t> {
  static constexpr auto datatype       = EXESS_SHORT;
  static constexpr auto read_function  = exess_read_short;
  static constexpr auto write_function = exess_write_short;
};

template<>
struct DatatypeTraits<int8_t> {
  static constexpr auto datatype       = EXESS_BYTE;
  static constexpr auto read_function  = exess_read_byte;
  static constexpr auto write_function = exess_write_byte;
};

template<>
struct DatatypeTraits<uint64_t> {
  static constexpr auto datatype       = EXESS_ULONG;
  static constexpr auto read_function  = exess_read_ulong;
  static constexpr auto write_function = exess_write_ulong;
};

template<>
struct DatatypeTraits<uint32_t> {
  static constexpr auto datatype       = EXESS_UINT;
  static constexpr auto read_function  = exess_read_uint;
  static constexpr auto write_function = exess_write_uint;
};

template<>
struct DatatypeTraits<uint16_t> {
  static constexpr auto datatype       = EXESS_USHORT;
  static constexpr auto read_function  = exess_read_ushort;
  static constexpr auto write_function = exess_write_ushort;
};

template<>
struct DatatypeTraits<uint8_t> {
  static constexpr auto datatype       = EXESS_UBYTE;
  static constexpr auto read_function  = exess_read_ubyte;
  static constexpr auto write_function = exess_write_ubyte;
};

template<>
struct DatatypeTraits<Duration> {
  static constexpr auto datatype       = EXESS_DURATION;
  static constexpr auto read_function  = exess_read_duration;
  static constexpr auto write_function = exess_write_duration;
};

template<>
struct DatatypeTraits<DateTime> {
  static constexpr auto datatype       = EXESS_DATETIME;
  static constexpr auto read_function  = exess_read_datetime;
  static constexpr auto write_function = exess_write_datetime;
};

template<>
struct DatatypeTraits<Time> {
  static constexpr auto datatype       = EXESS_TIME;
  static constexpr auto read_function  = exess_read_time;
  static constexpr auto write_function = exess_write_time;
};

template<>
struct DatatypeTraits<Date> {
  static constexpr auto datatype       = EXESS_DATE;
  static constexpr auto read_function  = exess_read_date;
  static constexpr auto write_function = exess_write_date;
};

} // namespace detail

/**
   @defgroup exesspp_read_write Reading and Writing
   @{
*/

/**
   Read a value from a string.

   @param out Set to the parsed value.
   @param str String input.
   @return The `count` of characters read, and a `status` code.
*/
template<class T>
inline Result
read(T* EXESS_NONNULL out, const char* EXESS_NONNULL str)
{
  return detail::DatatypeTraits<T>::read_function(out, str);
}

/**
   Write a value to a canonical string.

   @param value Value to write.
   @param buf_size The size of `buf` in bytes.
   @param buf Output buffer, or null to only measure.

   @return The `count` of characters in the output, and `status`
   Status::success, or Status::no_space if the buffer is too small.
*/
template<class T>
inline Result
write(const T& value, const size_t buf_size, char* EXESS_NONNULL buf)
{
  return detail::DatatypeTraits<T>::write_function(value, buf_size, buf);
}

/**
   Return a value as a string.

   This is a wrapper for write() that allocates a new string of the appropriate
   length, writes the value to it, and returns it.

   @param value The value to convert to a string.
   @return The value as a string, or the empty string on error.
*/
template<class T>
inline std::string
to_string(const T& value)
{
  auto r = detail::DatatypeTraits<T>::write_function(value, 0, nullptr);
  if (r.status) {
    return {};
  }

#if __cplusplus >= 201703L
  // In C++17, std::string::data() allows mutable access
  std::string string(r.count, ' ');
  r = detail::DatatypeTraits<T>::write_function(
    value, r.count + 1, string.data());

#else
  // Before, we had to allocate somewhere else
  std::vector<char> buf(r.count + 1, '\0');
  r = detail::DatatypeTraits<T>::write_function(value, r.count + 1, buf.data());

  std::string string(buf.data());
#endif

  return r.status ? "" : string;
}

/**
   @}
   @defgroup exesspp_datatypes Datatypes
   Runtime integer tags for supported datatypes with conversion to/from URIs.
   @{
*/

// enum class Datatype {
//   nothing,              ///< Sentinel for unknown datatypes or errors
//   boolean,              ///< xsd:boolean (see @ref exess_boolean)
//   decimal,              ///< xsd:decimal (see @ref exess_decimal)
//   double,               ///< xsd:double (see @ref exess_double)
//   float,                ///< xsd:float (see @ref exess_float)
//   integer,              ///< xsd:integer (see @ref exess_long)
//   non_positive_integer, ///< xsd:nonPositiveInteger (see @ref exess_long)
//   negative_integer,     ///< xsd:negativeInteger (see @ref exess_long)
//   long,                 ///< xsd:long (see @ref exess_long)
//   int,                  ///< xsd:integer (see @ref exess_int)
//   short,                ///< xsd:short (see @ref exess_short)
//   byte,                 ///< xsd:byte (see @ref exess_byte)
//   non_negative_integer, ///< xsd:nonNegativeInteger (see @ref exess_ulong)
//   ulong,                ///< xsd:unsignedLong (see @ref exess_ulong)
//   uint,                 ///< xsd:unsignedInt (see @ref exess_uint)
//   ushort,               ///< xsd:unsignedShort (see @ref exess_ushort)
//   ubyte,                ///< xsd:unsignedByte (see @ref exess_ubyte)
//   positive_integer,     ///< xsd:positiveInteger (see @ref exess_ulong)
//   duration,             ///< xsd:duration (see @ref exess_duration)
//   datetime,             ///< xsd:dateTime (see @ref exess_datetime)
//   time,                 ///< xsd:time (see @ref exess_time)
//   date,                 ///< xsd:date (see @ref exess_date)
//   hex,                  ///< xsd:hexBinary (see @ref exess_hex)
//   base64,               ///< xsd:base64Binary (see @ref exess_base64)
// }

/// @copydoc exess_datatype_uri
inline const char* EXESS_NULLABLE
datatype_uri(const Datatype datatype)
{
  return exess_datatype_uri(datatype);
}

/// @copydoc exess_datatype_from_uri
inline Datatype
datatype_from_uri(const char* const EXESS_NONNULL uri)
{
  return exess_datatype_from_uri(uri);
}

/// @copydoc exess_datatype_is_bounded
inline bool
datatype_is_bounded(const Datatype datatype)
{
  return exess_datatype_is_bounded(datatype);
}

/// @copydoc exess_max_length
inline size_t
max_length(const Datatype datatype)
{
  return exess_max_length(datatype);
}

template<class T>
constexpr size_t
max_length()
{
  return 0u;
}

template<>
constexpr size_t
max_length<bool>()
{
  return EXESS_MAX_BOOLEAN_LENGTH;
}

template<>
constexpr size_t
max_length<double>()
{
  return EXESS_MAX_DOUBLE_LENGTH;
}

template<>
constexpr size_t
max_length<float>()
{
  return EXESS_MAX_FLOAT_LENGTH;
}

template<>
constexpr size_t
max_length<int64_t>()
{
  return EXESS_MAX_LONG_LENGTH;
}

template<>
constexpr size_t
max_length<int32_t>()
{
  return EXESS_MAX_INT_LENGTH;
}

template<>
constexpr size_t
max_length<int16_t>()
{
  return EXESS_MAX_SHORT_LENGTH;
}

template<>
constexpr size_t
max_length<int8_t>()
{
  return EXESS_MAX_BYTE_LENGTH;
}

template<>
constexpr size_t
max_length<uint64_t>()
{
  return EXESS_MAX_ULONG_LENGTH;
}

template<>
constexpr size_t
max_length<uint32_t>()
{
  return EXESS_MAX_UINT_LENGTH;
}

template<>
constexpr size_t
max_length<uint16_t>()
{
  return EXESS_MAX_USHORT_LENGTH;
}

template<>
constexpr size_t
max_length<uint8_t>()
{
  return EXESS_MAX_UBYTE_LENGTH;
}

template<>
constexpr size_t
max_length<Duration>()
{
  return EXESS_MAX_DURATION_LENGTH;
}

template<>
constexpr size_t
max_length<DateTime>()
{
  return EXESS_MAX_DATETIME_LENGTH;
}

template<>
constexpr size_t
max_length<Time>()
{
  return EXESS_MAX_TIME_LENGTH;
}

template<>
constexpr size_t
max_length<Date>()
{
  return EXESS_MAX_DATE_LENGTH;
}

/**
   @}
   @}
*/

} // namespace exess

#endif // EXESS_EXESS_HPP
