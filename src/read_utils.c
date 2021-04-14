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

#include "read_utils.h"
#include "string_utils.h"

#include "exess/exess.h"

ExessResult
read_two_digit_number(uint8_t* const    out,
                      const uint8_t     min_value,
                      const uint8_t     max_value,
                      const char* const str)
{
  size_t i = 0;

  // Read digits
  size_t d = 0;
  for (; d < 2; ++d, ++i) {
    if (is_digit(str[i])) {
      *out = (uint8_t)((*out * 10) + (str[i] - '0'));
    } else {
      break;
    }
  }

  // Ensure there are exactly the expected number of digits
  if (d != 2) {
    return result(EXESS_EXPECTED_DIGIT, i);
  }

  // Ensure value is in range
  if (*out < min_value || *out > max_value) {
    return result(EXESS_OUT_OF_RANGE, i);
  }

  return result(EXESS_SUCCESS, i);
}
