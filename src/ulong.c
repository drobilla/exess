// Copyright 2019-2026 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#include <exess/exess.h>

#include "number_utils.h"
#include "read_utils.h"
#include "result.h"
#include "write_utils.h"

#include <stdint.h>
#include <stdlib.h>

EXESS_NONBLOCKING EXESS_NONBLOCKING ExessResult
exess_read_ulong(const char* const str, uint64_t* const out)
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

EXESS_NONBLOCKING EXESS_NONBLOCKING ExessResult
exess_write_ulong(const uint64_t value, const size_t buf_size, char* const buf)
{
  const ExessResult r = write_digits(value, buf_size, buf, 0);
  return end_write(r.status, buf_size, buf, r.count);
}
