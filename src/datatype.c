/*
  Copyright 2019-2021 David Robillard <d@drobilla.net>

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

#include "exess/exess.h"

#include <stdbool.h>
#include <string.h>

#define N_DATATYPES 24

/// The full URI for supported datatypes
static const char* EXESS_NONNULL const uris[N_DATATYPES + 1] = {
  NULL,                               //
  EXESS_XSD_URI "boolean",            //
  EXESS_XSD_URI "decimal",            //
  EXESS_XSD_URI "double",             //
  EXESS_XSD_URI "float",              //
  EXESS_XSD_URI "integer",            //
  EXESS_XSD_URI "nonPositiveInteger", //
  EXESS_XSD_URI "negativeInteger",    //
  EXESS_XSD_URI "long",               //
  EXESS_XSD_URI "int",                //
  EXESS_XSD_URI "short",              //
  EXESS_XSD_URI "byte",               //
  EXESS_XSD_URI "nonNegativeInteger", //
  EXESS_XSD_URI "unsignedLong",       //
  EXESS_XSD_URI "unsignedInt",        //
  EXESS_XSD_URI "unsignedShort",      //
  EXESS_XSD_URI "unsignedByte",       //
  EXESS_XSD_URI "positiveInteger",    //
  EXESS_XSD_URI "duration",           //
  EXESS_XSD_URI "datetime",           //
  EXESS_XSD_URI "time",               //
  EXESS_XSD_URI "date",               //
  EXESS_XSD_URI "hexBinary",          //
  EXESS_XSD_URI "base64Binary",       //
};

/// The maximum length of the string representation of datatypes
static const size_t exess_max_lengths[] = {
  0, // Unknown
  EXESS_MAX_BOOLEAN_LENGTH,
  0, // decimal
  EXESS_MAX_DOUBLE_LENGTH,
  EXESS_MAX_FLOAT_LENGTH,
  0, // integer
  0, // nonPositiveInteger
  0, // negativeInteger
  EXESS_MAX_LONG_LENGTH,
  EXESS_MAX_INT_LENGTH,
  EXESS_MAX_SHORT_LENGTH,
  EXESS_MAX_BYTE_LENGTH,
  0, // nonNegativeInteger
  EXESS_MAX_ULONG_LENGTH,
  EXESS_MAX_UINT_LENGTH,
  EXESS_MAX_USHORT_LENGTH,
  EXESS_MAX_UBYTE_LENGTH,
  0, // positiveInteger
  EXESS_MAX_DURATION_LENGTH,
  EXESS_MAX_DATETIME_LENGTH,
  EXESS_MAX_TIME_LENGTH,
  EXESS_MAX_DATE_LENGTH,
  0, // hexBinary
  0, // base64Binary
};

const char*
exess_datatype_uri(const ExessDatatype datatype)
{
  return (datatype > EXESS_NOTHING && datatype <= EXESS_BASE64) ? uris[datatype]
                                                                : NULL;
}

ExessDatatype
exess_datatype_from_uri(const char* const uri)
{
  static const size_t xsd_len = sizeof(EXESS_XSD_URI) - 1;

  if (!strncmp(uri, EXESS_XSD_URI, xsd_len)) {
    const char* const name = uri + xsd_len;
    for (size_t i = 1; i < N_DATATYPES; ++i) {
      if (!strcmp(name, uris[i] + xsd_len)) {
        return (ExessDatatype)i;
      }
    }
  }

  return EXESS_NOTHING;
}

bool
exess_datatype_is_bounded(const ExessDatatype datatype)
{
  return exess_max_length(datatype) > 0;
}

size_t
exess_max_length(const ExessDatatype datatype)
{
  return (datatype < N_DATATYPES) ? exess_max_lengths[datatype] : 0u;
}
