// Copyright 2019-2025 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#include <exess/exess.h>

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#define N_DATATYPES 24

static const size_t xsd_len = sizeof(EXESS_XSD_URI) - 1;

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

/// The size of the binary value representation of datatypes
static const size_t exess_value_sizes[] = {
  0, // Unknown
  sizeof(bool),
  sizeof(double),
  sizeof(double),
  sizeof(float),
  sizeof(int64_t),
  sizeof(int64_t),
  sizeof(int64_t),
  sizeof(int64_t),
  sizeof(int32_t),
  sizeof(int16_t),
  sizeof(int8_t),
  sizeof(uint64_t),
  sizeof(uint64_t),
  sizeof(uint32_t),
  sizeof(uint16_t),
  sizeof(uint8_t),
  sizeof(uint64_t),
  sizeof(ExessDuration),
  sizeof(ExessDateTime),
  sizeof(ExessTime),
  sizeof(ExessDate),
  0, // hexBinary
  0, // base64Binary
};

const char*
exess_datatype_uri(const ExessDatatype datatype)
{
  return (datatype > EXESS_NOTHING && datatype <= EXESS_BASE64) ? uris[datatype]
                                                                : NULL;
}

const char*
exess_datatype_name(const ExessDatatype datatype)
{
  const char* const uri = exess_datatype_uri(datatype);

  return uri ? uri + xsd_len : NULL;
}

ExessDatatype
exess_datatype_from_uri(const char* const uri)
{
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
  return (datatype < N_DATATYPES) ? exess_max_lengths[datatype] : 0U;
}

size_t
exess_value_size(const ExessDatatype datatype)
{
  return (datatype < N_DATATYPES) ? exess_value_sizes[datatype] : 0U;
}
