// Copyright 2019-2026 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#include <exess/exess.h>

static const char* const strings[] = {
  "Success",
  "Lossy operation",
  "Expected \"false\", \"true\", \"0\" or \"1\"",
  "Expected an integer value",
  "Expected a duration starting with 'P'",
  "Expected '-' or '+'",
  "Expected a digit",
  "Expected '0'",
  "Expected ':'",
  "Expected '-'",
  "Expected 'T'",
  "Expected 'H', 'M', or 'S'",
  "Expected 'Y', 'M', or 'D'",
  "Expected 'S'",
  "Expected a hexadecimal character",
  "Expected a base64 character",
  "Invalid field order",
  "Invalid value",
  "Value out of range",
  "Insufficient space",
  "Unsupported value",
};

EXESS_NONBLOCKING const char*
exess_strerror(const ExessStatus status)
{
  return ((unsigned)status <= (unsigned)EXESS_UNSUPPORTED) ? strings[status]
                                                           : "Unknown error";
}
