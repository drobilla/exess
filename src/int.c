// Copyright 2019-2021 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#include "result.h"

#include "exess/exess.h"

#include <stdint.h>
#include <stdlib.h>

ExessResult
exess_read_int(int32_t* const out, const char* const str)
{
  int64_t           long_out = 0;
  const ExessResult r        = exess_read_long(&long_out, str);
  if (r.status) {
    return r;
  }

  if (long_out < INT32_MIN || long_out > INT32_MAX) {
    return result(EXESS_OUT_OF_RANGE, r.count);
  }

  *out = (int32_t)long_out;
  return r;
}

ExessResult
exess_write_int(const int32_t value, const size_t buf_size, char* const buf)
{
  return exess_write_long(value, buf_size, buf);
}
