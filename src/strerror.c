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

const char*
exess_strerror(ExessStatus status)
{
  switch (status) {
  case EXESS_SUCCESS:
    return "Success";
  case EXESS_EXPECTED_END:
    return "Expected end of value";
  case EXESS_EXPECTED_BOOLEAN:
    return "Expected \"false\", \"true\", \"0\" or \"1\"";
  case EXESS_EXPECTED_INTEGER:
    return "Expected an integer value";
  case EXESS_EXPECTED_DURATION:
    return "Expected a duration starting with 'P'";
  case EXESS_EXPECTED_SIGN:
    return "Expected '-' or '+'";
  case EXESS_EXPECTED_DIGIT:
    return "Expected a digit";
  case EXESS_EXPECTED_COLON:
    return "Expected ':'";
  case EXESS_EXPECTED_DASH:
    return "Expected '-'";
  case EXESS_EXPECTED_TIME_SEP:
    return "Expected 'T'";
  case EXESS_EXPECTED_TIME_TAG:
    return "Expected 'H', 'M', or 'S'";
  case EXESS_EXPECTED_DATE_TAG:
    return "Expected 'Y', 'M', or 'D'";
  case EXESS_EXPECTED_HEX:
    return "Expected a hexadecimal character";
  case EXESS_EXPECTED_BASE64:
    return "Expected a base64 character";
  case EXESS_BAD_ORDER:
    return "Invalid field order";
  case EXESS_BAD_VALUE:
    return "Invalid value";
  case EXESS_OUT_OF_RANGE:
    return "Value outside valid range";
  case EXESS_NO_SPACE:
    return "Insufficient space";
  case EXESS_WOULD_REDUCE_PRECISION:
    return "Precision reducing coercion required";
  case EXESS_WOULD_ROUND:
    return "Rounding coercion required";
  case EXESS_WOULD_TRUNCATE:
    return "Truncating coercion required";
  case EXESS_UNSUPPORTED:
    return "Unsupported value";
  }

  return "Unknown error";
}
