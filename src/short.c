// Copyright 2019-2021 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#include "result.h"

#include "exess/exess.h"

#include <stdint.h>
#include <stdlib.h>

ExessResult
exess_read_short(int16_t* const out, const char* const str)
{
  int64_t           long_out = 0;
  const ExessResult r        = exess_read_long(&long_out, str);
  if (r.status) {
    return r;
  }

  if (long_out < INT16_MIN || long_out > INT16_MAX) {
    return result(EXESS_OUT_OF_RANGE, r.count);
  }

  *out = (int16_t)long_out;
  return r;
}

ExessResult
exess_write_short(const int16_t value, const size_t buf_size, char* const buf)
{
  return exess_write_long(value, buf_size, buf);
}
