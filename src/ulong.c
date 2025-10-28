// Copyright 2019-2025 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#include <exess/exess.h>

#include "int_math.h"
#include "number_utils.h"
#include "read_utils.h"
#include "result.h"
#include "write_utils.h"

#include <stdint.h>
#include <stdlib.h>

ExessResult
exess_read_ulong(uint64_t* const out, const char* const str)
{
  *out = 0;

  // Skip leading whitespace and read sign if present
  size_t i    = skip_whitespace(str);
  int    sign = 1;
  i += read_sign(&sign, &str[i]);

  if (sign == -1) {
    size_t j = i;
    while (str[j] == '0') {
      ++j;
    }

    return RESULT((j == i) ? EXESS_EXPECTED_ZERO : EXESS_SUCCESS, j);
  }

  ExessResult r = read_digits(out, str + i);

  r.count += i;
  return r;
}

ExessResult
exess_write_ulong(const uint64_t value, const size_t buf_size, char* const buf)
{
  if (!buf) {
    return RESULT(EXESS_SUCCESS, exess_num_digits(value));
  }

  const ExessResult r = write_digits(value, buf_size, buf, 0);
  return end_write(r.status, buf_size, buf, r.count);
}
