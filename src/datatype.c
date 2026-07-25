// Copyright 2019-2026 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#include <exess/exess.h>

#include <string.h>

static const size_t xsd_len = sizeof(EXESS_XSD_URI) - 1U;

/// The full URI for supported datatypes
static const char* EXESS_NONNULL const uris[EXESS_MAX_DATATYPE + 1U] = {
  NULL,                               //
  EXESS_XSD_URI "boolean",            //
  EXESS_XSD_URI "double",             //
  EXESS_XSD_URI "float",              //
  EXESS_XSD_URI "long",               //
  EXESS_XSD_URI "int",                //
  EXESS_XSD_URI "short",              //
  EXESS_XSD_URI "byte",               //
  EXESS_XSD_URI "unsignedLong",       //
  EXESS_XSD_URI "unsignedInt",        //
  EXESS_XSD_URI "unsignedShort",      //
  EXESS_XSD_URI "unsignedByte",       //
  EXESS_XSD_URI "decimal",            //
  EXESS_XSD_URI "integer",            //
  EXESS_XSD_URI "nonPositiveInteger", //
  EXESS_XSD_URI "negativeInteger",    //
  EXESS_XSD_URI "nonNegativeInteger", //
  EXESS_XSD_URI "positiveInteger",    //
  EXESS_XSD_URI "duration",           //
  EXESS_XSD_URI "yearMonthDuration",  //
  EXESS_XSD_URI "dayTimeDuration",    //
  EXESS_XSD_URI "dateTime",           //
  EXESS_XSD_URI "dateTimeStamp",      //
  EXESS_XSD_URI "time",               //
  EXESS_XSD_URI "date",               //
  EXESS_XSD_URI "gYearMonth",         //
  EXESS_XSD_URI "gYear",              //
  EXESS_XSD_URI "gMonthDay",          //
  EXESS_XSD_URI "gDay",               //
  EXESS_XSD_URI "gMonth",             //
  EXESS_XSD_URI "hexBinary",          //
  EXESS_XSD_URI "base64Binary",       //
};

EXESS_NONBLOCKING const char*
exess_datatype_uri(const ExessDatatype datatype)
{
  return (datatype > EXESS_NOTHING && datatype <= EXESS_BASE64) ? uris[datatype]
                                                                : NULL;
}

EXESS_NONBLOCKING const char*
exess_datatype_name(const ExessDatatype datatype)
{
  const char* const uri = exess_datatype_uri(datatype);

  return uri ? uri + xsd_len : NULL;
}

EXESS_NONBLOCKING ExessDatatype
exess_datatype_from_name(const char* const name)
{
  for (unsigned i = 1U; i <= EXESS_MAX_DATATYPE; ++i) {
    if (!strcmp(name, uris[i] + xsd_len)) {
      return (ExessDatatype)i;
    }
  }

  return EXESS_NOTHING;
}

EXESS_NONBLOCKING ExessDatatype
exess_datatype_from_uri(const char* const uri)
{
  return !strncmp(uri, EXESS_XSD_URI, xsd_len)
           ? exess_datatype_from_name(uri + xsd_len)
           : EXESS_NOTHING;
}
