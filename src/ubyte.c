/*
  Copyright 2019-2021 David Robillard <d@drobilla.net>

  Permission to use, copy, modify, and/or distribute this software for any
  purpose with or without fee is hereby granted, provided that the above
  copyright notice and this permission notice appear in all copies.

  THIS SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
  WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
  MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
  ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
  WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
  ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
  OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/

#include "read_utils.h"

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
