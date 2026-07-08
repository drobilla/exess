// Copyright 2019-2026 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#include "fixed.h"

#include <exess/exess.h>

#include <stdint.h>
#include <stdlib.h>

EXESS_NONBLOCKING ExessResult
exess_read_int(const char* const str, int32_t* const out)
{
  int64_t           value = 0;
  const ExessResult r     = read_signed(str, INT32_MIN, INT32_MAX, &value);

  if (!r.status) {
    *out = (int32_t)value;
  }

  return r;
}

EXESS_NONBLOCKING ExessResult
exess_write_int(const int32_t value, const size_t buf_size, char* const buf)
{
  return exess_write_long(value, buf_size, buf);
}
