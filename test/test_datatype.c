// Copyright 2011-2024 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#undef NDEBUG

#include <exess/exess.h>

#include <assert.h>
#include <stdbool.h>
#include <string.h>

static bool
datatype_uri_equals(const ExessDatatype datatype,
                    const char* const   expected_uri)
{
  const char* const uri = exess_datatype_uri(datatype);

  return uri && !strcmp(uri, expected_uri);
}

static void
test_datatype_uri(void)
{
  assert(!exess_datatype_uri(EXESS_NOTHING));
  assert(datatype_uri_equals(EXESS_BOOLEAN, EXESS_XSD_URI "boolean"));
  assert(datatype_uri_equals(EXESS_DECIMAL, EXESS_XSD_URI "decimal"));
  assert(datatype_uri_equals(EXESS_DOUBLE, EXESS_XSD_URI "double"));
  assert(datatype_uri_equals(EXESS_FLOAT, EXESS_XSD_URI "float"));
  assert(datatype_uri_equals(EXESS_INTEGER, EXESS_XSD_URI "integer"));

  assert(datatype_uri_equals(EXESS_NON_POSITIVE_INTEGER,
                             EXESS_XSD_URI "nonPositiveInteger"));
  assert(datatype_uri_equals(EXESS_NEGATIVE_INTEGER,
                             EXESS_XSD_URI "negativeInteger"));

  assert(datatype_uri_equals(EXESS_LONG, EXESS_XSD_URI "long"));
  assert(datatype_uri_equals(EXESS_INT, EXESS_XSD_URI "int"));
  assert(datatype_uri_equals(EXESS_SHORT, EXESS_XSD_URI "short"));
  assert(datatype_uri_equals(EXESS_BYTE, EXESS_XSD_URI "byte"));

  assert(datatype_uri_equals(EXESS_NON_NEGATIVE_INTEGER,
                             EXESS_XSD_URI "nonNegativeInteger"));

  assert(datatype_uri_equals(EXESS_ULONG, EXESS_XSD_URI "unsignedLong"));
  assert(datatype_uri_equals(EXESS_UINT, EXESS_XSD_URI "unsignedInt"));
  assert(datatype_uri_equals(EXESS_USHORT, EXESS_XSD_URI "unsignedShort"));
  assert(datatype_uri_equals(EXESS_UBYTE, EXESS_XSD_URI "unsignedByte"));

  assert(datatype_uri_equals(EXESS_POSITIVE_INTEGER,
                             EXESS_XSD_URI "positiveInteger"));

  assert(datatype_uri_equals(EXESS_DURATION, EXESS_XSD_URI "duration"));
  assert(datatype_uri_equals(EXESS_DATETIME, EXESS_XSD_URI "datetime"));
  assert(datatype_uri_equals(EXESS_TIME, EXESS_XSD_URI "time"));
  assert(datatype_uri_equals(EXESS_DATE, EXESS_XSD_URI "date"));

  assert(datatype_uri_equals(EXESS_HEX, EXESS_XSD_URI "hexBinary"));
  assert(datatype_uri_equals(EXESS_BASE64, EXESS_XSD_URI "base64Binary"));

  assert(!exess_datatype_uri((ExessDatatype)(EXESS_BASE64 + 1)));
}

static void
test_datatype_from_uri(void)
{
  assert(exess_datatype_from_uri(EXESS_XSD_URI) == EXESS_NOTHING);
  assert(exess_datatype_from_uri(EXESS_XSD_URI "unknown") == EXESS_NOTHING);
  assert(exess_datatype_from_uri("garbage") == EXESS_NOTHING);
  assert(exess_datatype_from_uri("http://example.org/very/long/unknown/uri") ==
         EXESS_NOTHING);
}

int
main(int argc, char** argv)
{
  (void)argv;

  test_datatype_uri();
  test_datatype_from_uri();

  return argc == 1 ? 0 : 1;
}
