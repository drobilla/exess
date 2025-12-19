// Copyright 2019-2025 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#include "fixed.h"

#include <exess/exess.h>

#include <stdint.h>
#include <stdlib.h>

ExessResult
exess_read_short(const char* const str, int16_t* const out)
{
  int64_t           value = 0;
  const ExessResult r     = read_signed(str, INT16_MIN, INT16_MAX, &value);

  if (!r.status) {
    *out = (int16_t)value;
  }

  return r;
}

ExessResult
exess_write_short(const int16_t value, const size_t buf_size, char* const buf)
{
  return exess_write_long(value, buf_size, buf);
}
