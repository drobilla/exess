// Copyright 2019-2021 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#include "result.h"

#include <exess/exess.h>

#include <stdint.h>
#include <stdlib.h>

ExessResult
exess_read_ushort(const char* const str, uint16_t* const out)
{
  uint64_t          long_out = 0;
  const ExessResult r        = exess_read_ulong(str, &long_out);
  if (r.status) {
    return r;
  }

  if (long_out > UINT16_MAX) {
    return RESULT(EXESS_OUT_OF_RANGE, r.count);
  }

  *out = (uint16_t)long_out;
  return r;
}

ExessResult
exess_write_ushort(const uint16_t value, const size_t buf_size, char* const buf)
{
  return exess_write_ulong(value, buf_size, buf);
}
