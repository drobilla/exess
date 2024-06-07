// Copyright 2011-2021 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#undef NDEBUG

#include "exess/exess.h"

#include <assert.h>
#include <string.h>

static void
test_datatype_uri(void)
{
  assert(!exess_datatype_uri(EXESS_NOTHING));
  assert(!strcmp(exess_datatype_uri(EXESS_BOOLEAN), EXESS_XSD_URI "boolean"));
  assert(!strcmp(exess_datatype_uri(EXESS_DECIMAL), EXESS_XSD_URI "decimal"));
  assert(!strcmp(exess_datatype_uri(EXESS_DOUBLE), EXESS_XSD_URI "double"));
  assert(!strcmp(exess_datatype_uri(EXESS_FLOAT), EXESS_XSD_URI "float"));
  assert(!strcmp(exess_datatype_uri(EXESS_INTEGER), EXESS_XSD_URI "integer"));

  assert(!strcmp(exess_datatype_uri(EXESS_NON_POSITIVE_INTEGER),
                 EXESS_XSD_URI "nonPositiveInteger"));
  assert(!strcmp(exess_datatype_uri(EXESS_NEGATIVE_INTEGER),
                 EXESS_XSD_URI "negativeInteger"));

  assert(!strcmp(exess_datatype_uri(EXESS_LONG), EXESS_XSD_URI "long"));
  assert(!strcmp(exess_datatype_uri(EXESS_INT), EXESS_XSD_URI "int"));
  assert(!strcmp(exess_datatype_uri(EXESS_SHORT), EXESS_XSD_URI "short"));
  assert(!strcmp(exess_datatype_uri(EXESS_BYTE), EXESS_XSD_URI "byte"));

  assert(!strcmp(exess_datatype_uri(EXESS_NON_NEGATIVE_INTEGER),
                 EXESS_XSD_URI "nonNegativeInteger"));

  assert(
    !strcmp(exess_datatype_uri(EXESS_ULONG), EXESS_XSD_URI "unsignedLong"));
  assert(!strcmp(exess_datatype_uri(EXESS_UINT), EXESS_XSD_URI "unsignedInt"));
  assert(
    !strcmp(exess_datatype_uri(EXESS_USHORT), EXESS_XSD_URI "unsignedShort"));
  assert(
    !strcmp(exess_datatype_uri(EXESS_UBYTE), EXESS_XSD_URI "unsignedByte"));

  assert(!strcmp(exess_datatype_uri(EXESS_POSITIVE_INTEGER),
                 EXESS_XSD_URI "positiveInteger"));

  assert(!strcmp(exess_datatype_uri(EXESS_DURATION), EXESS_XSD_URI "duration"));
  assert(!strcmp(exess_datatype_uri(EXESS_DATETIME), EXESS_XSD_URI "datetime"));
  assert(!strcmp(exess_datatype_uri(EXESS_TIME), EXESS_XSD_URI "time"));
  assert(!strcmp(exess_datatype_uri(EXESS_DATE), EXESS_XSD_URI "date"));

  assert(!strcmp(exess_datatype_uri(EXESS_HEX), EXESS_XSD_URI "hexBinary"));
  assert(
    !strcmp(exess_datatype_uri(EXESS_BASE64), EXESS_XSD_URI "base64Binary"));

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
