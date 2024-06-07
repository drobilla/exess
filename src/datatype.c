// Copyright 2019-2021 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#include <exess/exess.h>

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
