// Copyright 2019-2025 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#include "fixed.h"

#include <exess/exess.h>

#include <stdint.h>
#include <stdlib.h>

ExessResult
exess_read_byte(const char* const str, int8_t* const out)
{
  int64_t           value = 0;
  const ExessResult r     = read_signed(str, INT8_MIN, INT8_MAX, &value);

  if (!r.status) {
    *out = (int8_t)value;
  }

  return r;
}

ExessResult
exess_write_byte(const int8_t value, const size_t buf_size, char* const buf)
{
  return exess_write_long(value, buf_size, buf);
}
