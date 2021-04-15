/*
  Copyright 2021 David Robillard <d@drobilla.net>

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

#ifndef EXESS_EXESS_HPP
#define EXESS_EXESS_HPP

#include "exess/exess.h"

#include <cstdint>
#include <exception>
#include <iostream>
#include <string>
#include <type_traits>
#include <typeinfo>
#include <vector>

namespace exess {

/**
   @defgroup exesspp Exess C++ API
   This is the C++ wrapper for the exess API.
   @{
*/

constexpr const char* EXESS_NONNULL const xsd_uri =
  "http://www.w3.org/2001/XMLSchema#";

using Blob          = ExessBlob;
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
   @defgroup exesspp_variant Variant
   A Variant is a tagged union that can hold any supported datatype.
   @{
*/

class Variant : public ExessVariant
{
public:
  /// Construct a variant from an ExessVariant C structure
  explicit Variant(ExessVariant v)
    : ExessVariant{v}
  {}

  /// Construct a nothing variant with a status code
  explicit Variant(const Status status) noexcept
    : ExessVariant{}
  {
    datatype        = EXESS_NOTHING;
    value.as_status = static_cast<ExessStatus>(status);
  }

  /// Construct a boolean variant
  explicit Variant(const bool v) noexcept
    : ExessVariant{}
  {
    datatype      = EXESS_BOOLEAN;
    value.as_bool = v;
  }

  /// Construct a double variant
  explicit Variant(const double v) noexcept
    : ExessVariant{}
  {
    datatype        = EXESS_DOUBLE;
    value.as_double = v;
  }

  /// Construct a float variant
  explicit Variant(const float v) noexcept
    : ExessVariant{}
  {
    datatype       = EXESS_FLOAT;
    value.as_float = v;
  }

  /// Construct a long variant
  explicit Variant(const int64_t v) noexcept
    : ExessVariant{}
  {
    datatype      = EXESS_LONG;
    value.as_long = v;
  }

  /// Construct an int variant
  explicit Variant(const int32_t v) noexcept
    : ExessVariant{}
  {
    datatype     = EXESS_INT;
    value.as_int = v;
  }

  /// Construct a short variant
  explicit Variant(const int16_t v) noexcept
    : ExessVariant{}
  {
    datatype       = EXESS_SHORT;
    value.as_short = v;
  }

  /// Construct a byte variant
  explicit Variant(const int8_t v) noexcept
    : ExessVariant{}
  {
    datatype      = EXESS_BYTE;
    value.as_byte = v;
  }

  /// Construct an unsigned long variant
  explicit Variant(const uint64_t v) noexcept
    : ExessVariant{}
  {
    datatype       = EXESS_ULONG;
    value.as_ulong = v;
  }

  /// Construct an unsigned int variant
  explicit Variant(const uint32_t v) noexcept
    : ExessVariant{}
  {
    datatype      = EXESS_UINT;
    value.as_uint = v;
  }

  /// Construct an unsigned short variant
  explicit Variant(const uint16_t v) noexcept
    : ExessVariant{}
  {
    datatype        = EXESS_USHORT;
    value.as_ushort = v;
  }

  /// Construct an unsigned byte variant
  explicit Variant(const uint8_t v) noexcept
    : ExessVariant{}
  {
    datatype       = EXESS_UBYTE;
    value.as_ubyte = v;
  }

  /// Construct a duration variant
  explicit Variant(const Duration v) noexcept
    : ExessVariant{}
  {
    datatype          = EXESS_DURATION;
    value.as_duration = v;
  }

  /// Construct a datetime variant
  explicit Variant(const DateTime v) noexcept
    : ExessVariant{}
  {
    datatype          = EXESS_DATETIME;
    value.as_datetime = v;
  }

  /// Construct a time variant
  explicit Variant(const Time v) noexcept
    : ExessVariant{}
  {
    datatype      = EXESS_TIME;
    value.as_time = v;
  }

  /// Construct a date variant
  explicit Variant(const Date v) noexcept
    : ExessVariant{}
  {
    datatype      = EXESS_DATE;
    value.as_date = v;
  }
};

/**
   @defgroup exesspp_variant_construction Construction
   @{
*/

inline Variant
make_decimal(const double value) noexcept
{
  Variant result{value};
  result.datatype = EXESS_DECIMAL;
  return result;
}

inline Variant
make_integer(const int64_t value) noexcept
{
  Variant result{value};
  result.datatype = EXESS_INTEGER;
  return result;
}

inline Variant
make_non_positive_integer(const int64_t value) noexcept
{
  if (value > 0) {
    return Variant{Status::out_of_range};
  }

  Variant result{value};
  result.datatype = EXESS_NON_POSITIVE_INTEGER;
  return result;
}

inline Variant
make_negative_integer(const int64_t value) noexcept
{
  if (value >= 0) {
    return Variant{Status::out_of_range};
  }

  Variant result{value};
  result.datatype = EXESS_NEGATIVE_INTEGER;
  return result;
}

inline Variant
make_non_negative_integer(const uint64_t value) noexcept
{
  Variant result{value};
  result.datatype = EXESS_NON_NEGATIVE_INTEGER;
  return result;
}

inline Variant
make_positive_integer(const uint64_t value) noexcept
{
  if (value == 0) {
    return Variant{Status::out_of_range};
  }

  Variant result{value};
  result.datatype = EXESS_POSITIVE_INTEGER;
  return result;
}

inline Variant
make_hex(const size_t size, void* const EXESS_NONNULL data) noexcept
{
  Variant result{Status::success};

  result.datatype           = EXESS_HEX;
  result.value.as_blob.size = size;
  result.value.as_blob.data = data;

  return result;
}

inline Variant
make_base64(const size_t size, void* const EXESS_NONNULL data) noexcept
{
  Variant result{Status::success};

  result.datatype           = EXESS_BASE64;
  result.value.as_blob.size = size;
  result.value.as_blob.data = data;

  return result;
}

/**
   @}
   @defgroup exesspp_variant_read_write Reading and Writing
   @{
*/

/**
   Read any supported datatype from a string.

   For reading binary blobs from base64 or hex, `out` should be an allocated
   blob with the size set to the size of the available buffer in bytes.  On
   return, the size will be set to the exact size of the decoded data, which
   may be smaller than the initial available size.  Only these first bytes are
   written, the rest of the buffer is not modified.

   @param out Set to the parsed value, or nothing on error.
   @param datatype The datatype to read the string as.
   @param str String input.
   @return The `count` of characters read, and a `status` code.
*/
inline Result
read_variant(Variant* EXESS_NONNULL    out,
             const Datatype            datatype,
             const char* EXESS_NONNULL str)
{
  return static_cast<Result>(
    exess_read_variant(out, static_cast<ExessDatatype>(datatype), str));
}

/**
   Write any supported xsd datatype to a canonical string.

   @param value Value to write.
   @param buf_size The size of `buf` in bytes.
   @param buf Output buffer, or null to only measure.

   @return The `count` of characters in the output, and `status`
   #EXESS_SUCCESS, or #EXESS_NO_SPACE if the buffer is too small.
*/
inline Result
write_variant(const Variant&       value,
              const size_t         buf_size,
              char* EXESS_NULLABLE buf)
{
  return static_cast<Result>(exess_write_variant(value, buf_size, buf));
}

/// @copydoc exess_write_canonical
inline Result
write_canonical(const char* const EXESS_NONNULL value,
                const Datatype                  datatype,
                const size_t                    buf_size,
                char* const EXESS_NULLABLE      buf)
{
  return exess_write_canonical(value, datatype, buf_size, buf);
}

/**
   Return a value as a string.

   This is a wrapper for write() that allocates a new string of the appropriate
   length, writes the value to it, and returns it.

   @param value The value to convert to a string.
   @param datatype Datatype of value.
   @return The value as a canonical string, or the empty string on error.
*/
inline std::string
canonical_string(const char* const EXESS_NONNULL value, const Datatype datatype)
{
  auto r = write_canonical(value, datatype, 0, nullptr);
  if (r.status) {
    return {};
  }

#if __cplusplus >= 201703L
  // In C++17, std::string::data() allows mutable access
  std::string string(r.count, ' ');
  r = write_canonical(value, datatype, r.count + 1, string.data());

  return r.status ? "" : string;

#else
  // Before, we had to allocate somewhere else
  std::vector<char> buf(r.count + 1, '\0');
  r = exess_write_canonical(value, datatype, r.count + 1, buf.data());

  return r.status ? "" : std::string(buf.data());
#endif
}

/**
   @}
   @defgroup exesspp_variant_access Access
   @{
*/

/**
   Return a pointer to the value of `variant` if it has type `T`.

   This is safe to call on any variant, and will only return a pointer to the
   value if the variant has the matching type and the value is valid to read.

   @return A pointer to the value in `variant`, or null.
*/
template<class T>
constexpr const T* EXESS_NULLABLE
get_if(const Variant& variant) noexcept
{
  return (variant.datatype == detail::DatatypeTraits<T>::datatype)
           ? reinterpret_cast<const T*>(&variant.value)
           : nullptr;
}

/**
   Return a pointer to the Status value in `variant` if it exists.
*/
template<>
constexpr const Status* EXESS_NULLABLE
get_if<Status>(const Variant& variant) noexcept
{
  return variant.datatype == EXESS_NOTHING
           ? reinterpret_cast<const Status*>(&variant.value.as_status)
           : nullptr;
}

/**
   Return a pointer to the value of `variant` if it is a `double`.

   This specialization works for both #EXESS_DECIMAL and #EXESS_DOUBLE values.
*/
template<>
constexpr const double* EXESS_NULLABLE
get_if<double>(const Variant& variant) noexcept
{
  return (variant.datatype == EXESS_DECIMAL || variant.datatype == EXESS_DOUBLE)
           ? &variant.value.as_double
           : nullptr;
}

/**
   Return a pointer to the value of `variant` if it is an `int64_t` (long).

   This specialization works for #EXESS_INTEGER, #EXESS_NON_POSITIVE_INTEGER,
   #EXESS_NEGATIVE_INTEGER, and #EXESS_LONG values.
*/
template<>
constexpr const int64_t* EXESS_NULLABLE
get_if<int64_t>(const Variant& variant) noexcept
{
  return (variant.datatype >= EXESS_INTEGER && variant.datatype <= EXESS_LONG)
           ? &variant.value.as_long
           : nullptr;
}

/**
   Return a pointer to the value of `variant` if it is a `uint64_t` (ulong).

   This specialization works for #EXESS_NON_NEGATIVE_INTEGER, #EXESS_ULONG, and
   #EXESS_POSITIVE_INTEGER values.
*/
template<>
constexpr const uint64_t* EXESS_NULLABLE
get_if<uint64_t>(const Variant& variant) noexcept
{
  return (variant.datatype == EXESS_NON_NEGATIVE_INTEGER ||
          variant.datatype == EXESS_ULONG ||
          variant.datatype == EXESS_POSITIVE_INTEGER)
           ? &variant.value.as_ulong
           : nullptr;
}

/**
   Return a pointer to the value of `variant` if it is a `uint64_t` (ulong).

   This specialization works for #EXESS_NON_NEGATIVE_INTEGER, #EXESS_ULONG, and
   #EXESS_POSITIVE_INTEGER values.
*/
template<>
constexpr const Blob* EXESS_NULLABLE
get_if<Blob>(const Variant& variant) noexcept
{
  return (variant.datatype == EXESS_HEX || variant.datatype == EXESS_BASE64)
           ? &variant.value.as_blob
           : nullptr;
}

/// Return the value of a variant with the given type
template<class T>
const T&
get(const Variant& variant)
{
  if (const T* const pointer = get_if<T>(variant)) {
    return *pointer;
  }

  if (variant.datatype == EXESS_NOTHING) {
    throw std::runtime_error{"Empty exess::Variant access"};
  }

  throw std::runtime_error{
    std::string("Bad exess::Variant access: ") +
    std::string(exess_datatype_uri(variant.datatype) + strlen(xsd_uri)) +
    " from " + typeid(T).name()};
}

/**
   @}
   @defgroup exesspp_coercion Datatype Coercion

   Values can be converted between some datatypes using coerce().  This is
   particularly useful for reducing the number of datatypes that the application
   needs to explicitly handle.

   @{
*/

/**
   Coerce a value to another datatype if possible.

   @param value Value to coerce.

   @param datatype Datatype to convert to.

   @param coercions Enabled coercion flags.  If this is #EXESS_LOSSLESS (zero),
   then #EXESS_SUCCESS is only returned if the resulting value can be coerced
   back to the original type without any loss of data.  Otherwise, the lossy
   coercions enabled by the set bits will be attempted.

   @return #EXESS_SUCCESS on successful conversion, #EXESS_OUT_OF_RANGE if the
   value is outside the range of the target type,
   #EXESS_WOULD_REDUCE_PRECISION, #EXESS_WOULD_ROUND, or #EXESS_WOULD_TRUNCATE
   if the required coercion is not enabled, or #EXESS_UNSUPPORTED if conversion
   between the types is not supported at all.
*/
inline Variant
coerce(const Variant&      value,
       const Datatype      datatype,
       const CoercionFlags coercions)
{
  return static_cast<Variant>(exess_coerce(value, datatype, coercions));
}

/**
   @}
   @}
*/

} // namespace exess

#endif // EXESS_EXESS_HPP
