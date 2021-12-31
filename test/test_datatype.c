// Copyright 2011-2021 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#undef NDEBUG

#include "exess/exess.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

static void
test_datatype_uris(void)
{
  assert(!exess_datatype_uri(EXESS_NOTHING));
  assert(!exess_datatype_uri((ExessDatatype)(EXESS_BASE64 + 1)));

  for (unsigned i = 1; i <= EXESS_BASE64; ++i) {
    const char* const uri = exess_datatype_uri((ExessDatatype)i);

    assert(uri);
    assert(exess_datatype_from_uri(uri) == (ExessDatatype)i);
  }

  assert(exess_datatype_from_uri(EXESS_XSD_URI) == EXESS_NOTHING);
  assert(exess_datatype_from_uri(EXESS_XSD_URI "unknown") == EXESS_NOTHING);
  assert(exess_datatype_from_uri("garbage") == EXESS_NOTHING);
  assert(exess_datatype_from_uri("http://example.org/very/long/unknown/uri") ==
         EXESS_NOTHING);
}

static void
test_datatype_is_bounded(void)
{
  assert(!exess_datatype_is_bounded(EXESS_NOTHING));
  assert(exess_datatype_is_bounded(EXESS_BOOLEAN));
  assert(!exess_datatype_is_bounded(EXESS_DECIMAL));
  assert(exess_datatype_is_bounded(EXESS_DOUBLE));
  assert(exess_datatype_is_bounded(EXESS_FLOAT));
  assert(!exess_datatype_is_bounded(EXESS_INTEGER));
  assert(!exess_datatype_is_bounded(EXESS_NON_POSITIVE_INTEGER));
  assert(!exess_datatype_is_bounded(EXESS_NEGATIVE_INTEGER));
  assert(exess_datatype_is_bounded(EXESS_LONG));
  assert(exess_datatype_is_bounded(EXESS_INT));
  assert(exess_datatype_is_bounded(EXESS_SHORT));
  assert(exess_datatype_is_bounded(EXESS_BYTE));
  assert(!exess_datatype_is_bounded(EXESS_NON_NEGATIVE_INTEGER));
  assert(exess_datatype_is_bounded(EXESS_ULONG));
  assert(exess_datatype_is_bounded(EXESS_UINT));
  assert(exess_datatype_is_bounded(EXESS_USHORT));
  assert(exess_datatype_is_bounded(EXESS_UBYTE));
  assert(!exess_datatype_is_bounded(EXESS_POSITIVE_INTEGER));
  assert(exess_datatype_is_bounded(EXESS_DURATION));
  assert(exess_datatype_is_bounded(EXESS_DATETIME));
  assert(exess_datatype_is_bounded(EXESS_TIME));
  assert(exess_datatype_is_bounded(EXESS_DATE));
  assert(!exess_datatype_is_bounded(EXESS_HEX));
  assert(!exess_datatype_is_bounded(EXESS_BASE64));
}

static void
test_value_size(void)
{
  assert(exess_value_size((ExessDatatype)-1) == 0u);

  assert(exess_value_size(EXESS_NOTHING) == 0u);
  assert(exess_value_size(EXESS_BOOLEAN) == sizeof(bool));
  assert(exess_value_size(EXESS_DECIMAL) == sizeof(double));
  assert(exess_value_size(EXESS_DOUBLE) == sizeof(double));
  assert(exess_value_size(EXESS_FLOAT) == sizeof(float));
  assert(exess_value_size(EXESS_INTEGER) == sizeof(int64_t));
  assert(exess_value_size(EXESS_NON_POSITIVE_INTEGER) == sizeof(int64_t));
  assert(exess_value_size(EXESS_NEGATIVE_INTEGER) == sizeof(int64_t));
  assert(exess_value_size(EXESS_LONG) == sizeof(int64_t));
  assert(exess_value_size(EXESS_INT) == sizeof(int32_t));
  assert(exess_value_size(EXESS_SHORT) == sizeof(int16_t));
  assert(exess_value_size(EXESS_BYTE) == sizeof(int8_t));
  assert(exess_value_size(EXESS_NON_NEGATIVE_INTEGER) == sizeof(uint64_t));
  assert(exess_value_size(EXESS_ULONG) == sizeof(uint64_t));
  assert(exess_value_size(EXESS_UINT) == sizeof(uint32_t));
  assert(exess_value_size(EXESS_USHORT) == sizeof(uint16_t));
  assert(exess_value_size(EXESS_UBYTE) == sizeof(uint8_t));
  assert(exess_value_size(EXESS_POSITIVE_INTEGER) == sizeof(uint64_t));
  assert(exess_value_size(EXESS_DURATION) == sizeof(ExessDuration));
  assert(exess_value_size(EXESS_DATETIME) == sizeof(ExessDateTime));
  assert(exess_value_size(EXESS_TIME) == sizeof(ExessTime));
  assert(exess_value_size(EXESS_DATE) == sizeof(ExessDate));
  assert(exess_value_size(EXESS_HEX) == 0u);
  assert(exess_value_size(EXESS_BASE64) == 0u);
}

int
main(int argc, char** argv)
{
  (void)argv;

  test_datatype_uris();
  test_datatype_is_bounded();
  test_value_size();

  return argc == 1 ? 0 : 1;
}
