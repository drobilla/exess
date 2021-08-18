// Copyright 2021 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#undef NDEBUG

#include "float_test_data.h"

#include "exess/exess.h"
#include "exess/exess.hpp"

#include <array>
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

namespace exess {
namespace {

void
test_status()
{
  std::ostringstream ss;
  ss << Status::no_space;

  std::cerr << ss.str();
  assert(ss.str() == "Insufficient space");
}

void
test_read()
{
  bool a_bool = false;
  assert(!read(&a_bool, "true").status);
  assert(a_bool == true);

  double a_double = 0.0;
  assert(!read(&a_double, "4.2E16").status);
  assert(double_matches(a_double, 42000000000000000.0));

  float a_float = 0.0f;
  assert(!read(&a_float, "4.2E7").status);
  assert(float_matches(a_float, 42000000.0f));

  int64_t a_long = 0;
  assert(!read(&a_long, "-4200000000").status);
  assert(a_long == -4200000000LL);

  int32_t a_int = 0;
  assert(!read(&a_int, "-42000").status);
  assert(a_int == -42000);

  int16_t a_short = 0;
  assert(!read(&a_short, "-420").status);
  assert(a_short == -420);

  int8_t a_byte = 0;
  assert(!read(&a_byte, "-42").status);
  assert(a_byte == -42);

  uint64_t a_ulong = 0u;
  assert(!read(&a_ulong, "4200000000").status);
  assert(a_ulong == 4200000000);

  uint32_t a_uint = 0u;
  assert(!read(&a_uint, "42000").status);
  assert(a_uint == 42000);

  uint16_t a_ushort = 0u;
  assert(!read(&a_ushort, "420").status);
  assert(a_ushort == 420);

  uint8_t a_ubyte = 0u;
  assert(!read(&a_ubyte, "42").status);
  assert(a_ubyte == 42);
}

void
test_to_string()
{
  assert(to_string(true) == "true");
  assert(to_string(42000000000000000.0) == "4.2E16");
  assert(to_string(42000000.0f) == "4.2E7");
  assert(to_string(int64_t(-4200000000LL)) == "-4200000000");
  assert(to_string(int32_t(-42000)) == "-42000");
  assert(to_string(int16_t(-420)) == "-420");
  assert(to_string(int8_t(-42)) == "-42");
  assert(to_string(uint64_t(4200000000u)) == "4200000000");
  assert(to_string(uint32_t(42000u)) == "42000");
  assert(to_string(uint16_t(420u)) == "420");
  assert(to_string(uint8_t(42u)) == "42");
}

void
test_max_length()
{
  static_assert(max_length<bool>() == EXESS_MAX_BOOLEAN_LENGTH, "");
  static_assert(max_length<double>() == EXESS_MAX_DOUBLE_LENGTH, "");
  static_assert(max_length<float>() == EXESS_MAX_FLOAT_LENGTH, "");
  static_assert(max_length<int64_t>() == EXESS_MAX_LONG_LENGTH, "");
  static_assert(max_length<int32_t>() == EXESS_MAX_INT_LENGTH, "");
  static_assert(max_length<int16_t>() == EXESS_MAX_SHORT_LENGTH, "");
  static_assert(max_length<int8_t>() == EXESS_MAX_BYTE_LENGTH, "");
  static_assert(max_length<uint64_t>() == EXESS_MAX_ULONG_LENGTH, "");
  static_assert(max_length<uint32_t>() == EXESS_MAX_UINT_LENGTH, "");
  static_assert(max_length<uint16_t>() == EXESS_MAX_USHORT_LENGTH, "");
  static_assert(max_length<uint8_t>() == EXESS_MAX_UBYTE_LENGTH, "");
  static_assert(max_length<Duration>() == EXESS_MAX_DURATION_LENGTH, "");
  static_assert(max_length<DateTime>() == EXESS_MAX_DATETIME_LENGTH, "");
  static_assert(max_length<Date>() == EXESS_MAX_DATE_LENGTH, "");
  static_assert(max_length<Time>() == EXESS_MAX_TIME_LENGTH, "");
}

template<class T>
void
check_get_throws(const Variant& variant)
{
  bool caught = false;

  try {
    get<T>(variant);
  } catch (const std::runtime_error&) {
    caught = true;
  }

  assert(caught);
}

void
test_variant()
{
  std::array<char, 4> blob_value{'b', 'l', 'o', 'b'};

  const auto a_nothing              = Variant{Status::success};
  const auto a_bool                 = Variant{true};
  const auto a_decimal              = make_decimal(1.2);
  const auto a_double               = Variant{3.4};
  const auto a_float                = Variant{5.6f};
  const auto a_integer              = make_integer(7);
  const auto a_non_positive_integer = make_non_positive_integer(-8);
  const auto a_negative_integer     = make_negative_integer(-9);
  const auto a_long                 = Variant{int64_t(10)};
  const auto a_int                  = Variant{int32_t(11)};
  const auto a_short                = Variant{int16_t(12)};
  const auto a_byte                 = Variant{int8_t(13)};
  const auto a_non_negative_integer = make_non_negative_integer(14u);
  const auto a_ulong                = Variant{uint64_t(15u)};
  const auto a_uint                 = Variant{uint32_t(16u)};
  const auto a_ushort               = Variant{uint16_t(17u)};
  const auto a_ubyte                = Variant{uint8_t(18u)};
  const auto a_positive_integer     = make_positive_integer(19u);

  const auto a_hex    = make_hex(blob_value.size(), blob_value.data());
  const auto a_base64 = make_base64(blob_value.size(), blob_value.data());

  try {
    assert(get<Status>(a_nothing) == Status::success);
    assert(get<bool>(a_bool) == true);
    assert(double_matches(get<double>(a_decimal), 1.2));
    assert(double_matches(get<double>(a_double), 3.4));
    assert(float_matches(get<float>(a_float), 5.6f));
    assert(get<int64_t>(a_integer) == 7);
    assert(get<int64_t>(a_non_positive_integer) == -8);
    assert(get<int64_t>(a_negative_integer) == -9);
    assert(get<int64_t>(a_long) == 10);
    assert(get<int32_t>(a_int) == 11);
    assert(get<int16_t>(a_short) == 12);
    assert(get<int8_t>(a_byte) == 13);
    assert(get<uint64_t>(a_non_negative_integer) == 14u);
    assert(get<uint64_t>(a_ulong) == 15u);
    assert(get<uint32_t>(a_uint) == 16u);
    assert(get<uint16_t>(a_ushort) == 17u);
    assert(get<uint8_t>(a_ubyte) == 18u);
    assert(get<uint64_t>(a_positive_integer) == 19u);
    assert(get<Blob>(a_hex).size == sizeof(blob_value));
    assert(
      !memcmp(get<Blob>(a_hex).data, blob_value.data(), blob_value.size()));
    assert(get<Blob>(a_base64).size == sizeof(blob_value));
    assert(
      !memcmp(get<Blob>(a_base64).data, blob_value.data(), blob_value.size()));
  } catch (const std::runtime_error&) {
    abort();
  }

  check_get_throws<int>(a_nothing);
  check_get_throws<int>(a_bool);
  check_get_throws<int>(a_double);
  check_get_throws<int>(a_float);
  check_get_throws<int>(a_integer);
  check_get_throws<int>(a_non_positive_integer);
  check_get_throws<int>(a_negative_integer);
  check_get_throws<int>(a_long);
  check_get_throws<bool>(a_int);
  check_get_throws<int>(a_short);
  check_get_throws<int>(a_byte);
  check_get_throws<int>(a_non_negative_integer);
  check_get_throws<int>(a_ulong);
  check_get_throws<int>(a_uint);
  check_get_throws<int>(a_ushort);
  check_get_throws<int>(a_ubyte);
  check_get_throws<int>(a_positive_integer);

  auto variant = exess::Variant{0};
  assert(!read_variant(&variant, EXESS_INT, "1234").status);
  assert(exess::get<int32_t>(variant) == 1234);
}

} // namespace
} // namespace exess

int
main()
{
  try {
    exess::test_status();
    exess::test_read();
    exess::test_to_string();
    exess::test_max_length();
    exess::test_variant();
  } catch (...) {
    return 1;
  }

  return 0;
}
