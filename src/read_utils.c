// Copyright 2019-2021 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#include "read_utils.h"

#include "result.h"
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
