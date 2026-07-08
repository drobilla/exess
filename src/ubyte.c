// Copyright 2019-2026 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#include "fixed.h"

#include <exess/exess.h>

#include <stdint.h>
#include <stdlib.h>

EXESS_NONBLOCKING EXESS_NONBLOCKING ExessResult
exess_read_ubyte(const char* const str, uint8_t* const out)
{
  uint64_t          value = 0U;
  const ExessResult r     = read_unsigned(str, UINT8_MAX, &value);

  *out = (uint8_t)value;
  return r;
}

EXESS_NONBLOCKING EXESS_NONBLOCKING ExessResult
exess_write_ubyte(const uint8_t value, const size_t buf_size, char* const buf)
{
  return exess_write_ulong(value, buf_size, buf);
}
