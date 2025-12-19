// Copyright 2019-2025 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#include "fixed.h"

#include <exess/exess.h>

#include <stdint.h>
#include <stdlib.h>

ExessResult
exess_read_ushort(const char* const str, uint16_t* const out)
{
  uint64_t          value = 0U;
  const ExessResult r     = read_unsigned(str, UINT16_MAX, &value);

  *out = (uint16_t)value;
  return r;
}

ExessResult
exess_write_ushort(const uint16_t value, const size_t buf_size, char* const buf)
{
  return exess_write_ulong(value, buf_size, buf);
}
