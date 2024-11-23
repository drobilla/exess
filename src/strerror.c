// Copyright 2019-2021 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#include <exess/exess.h>

const char*
exess_strerror(const ExessStatus status)
{
  static const char* const strings[] = {
    "Success",
    "Expected end of value",
    "Expected \"false\", \"true\", \"0\" or \"1\"",
    "Expected an integer value",
    "Expected a duration starting with 'P'",
    "Expected '-' or '+'",
    "Expected a digit",
    "Expected ':'",
    "Expected '-'",
    "Expected 'T'",
    "Expected 'H', 'M', or 'S'",
    "Expected 'Y', 'M', or 'D'",
    "Expected a hexadecimal character",
    "Expected a base64 character",
    "Invalid field order",
    "Invalid value",
    "Value outside valid range",
    "Insufficient space",
    "Precision reducing coercion required",
    "Rounding coercion required",
    "Truncating coercion required",
    "Unsupported value",
  };

  return ((unsigned)status <= (unsigned)EXESS_UNSUPPORTED) ? strings[status]
                                                           : "Unknown error";
}
