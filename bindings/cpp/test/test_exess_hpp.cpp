// Copyright 2021 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#undef NDEBUG

#include "float_test_data.h"

#include "exess/exess.h"
#include "exess/exess.hpp"

#include <cassert>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <sstream>
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
test_datatype()
{
  assert(!strcmp(datatype_uri(Datatype::Boolean),
                 "http://www.w3.org/2001/XMLSchema#boolean"));

  assert(datatype_from_uri("http://www.w3.org/2001/XMLSchema#boolean") ==
         Datatype::Boolean);

  for (auto i = 1u; i <= unsigned(EXESS_BASE64); ++i) {
    const auto        datatype = static_cast<Datatype>(i);
    const char* const uri      = datatype_uri(datatype);

    assert(uri && datatype_from_uri(uri) == datatype);
  }
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

} // namespace
} // namespace exess

int
main()
{
  try {
    exess::test_status();
    exess::test_datatype();
    exess::test_read();
    exess::test_to_string();
    exess::test_max_length();
  } catch (...) {
    return 1;
  }

  return 0;
}
