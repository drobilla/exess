// Copyright 2019-2025 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#include "fixed.h"

#include <exess/exess.h>

#include <stdint.h>
#include <stdlib.h>

ExessResult
exess_read_uint(const char* const str, uint32_t* const out)
{
  uint64_t          value = 0U;
  const ExessResult r     = read_unsigned(str, UINT32_MAX, &value);

  *out = (uint32_t)value;
  return r;
}

ExessResult
exess_write_uint(const uint32_t value, const size_t buf_size, char* const buf)
{
  return exess_write_ulong(value, buf_size, buf);
}
