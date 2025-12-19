// Copyright 2025 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#include "fixed.h"
#include "result.h"

#include <exess/exess.h>

#include <stdint.h>

ExessResult
read_signed(const char* const str,
            const int64_t     min,
            const int64_t     max,
            int64_t* const    out)
{
  const ExessResult r = exess_read_long(str, out);

  return (!r.status && (*out < min || *out > max))
           ? RESULT(EXESS_OUT_OF_RANGE, r.count)
           : r;
}

ExessResult
read_unsigned(const char* const str, const uint64_t max, uint64_t* const out)
{
  const ExessResult r = exess_read_ulong(str, out);

  return (!r.status && *out > max) ? RESULT(EXESS_OUT_OF_RANGE, r.count) : r;
}
