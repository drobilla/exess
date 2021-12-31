// Copyright 2019-2021 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#include "result.h"

#include "exess/exess.h"

#include <stdint.h>
#include <stdlib.h>

ExessResult
exess_read_ubyte(uint8_t* const out, const char* const str)
{
  uint64_t          long_out = 0;
  const ExessResult r        = exess_read_ulong(&long_out, str);
  if (r.status) {
    return r;
  }

  if (long_out > UINT8_MAX) {
    return result(EXESS_OUT_OF_RANGE, r.count);
  }

  *out = (uint8_t)long_out;
  return r;
}

ExessResult
exess_write_ubyte(const uint8_t value, const size_t buf_size, char* const buf)
{
  return exess_write_ulong(value, buf_size, buf);
}
