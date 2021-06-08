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

#ifndef EXESS_TIME_UTILS_H
#define EXESS_TIME_UTILS_H

#include "exess/exess.h"

#include <stddef.h>
#include <stdint.h>

/// Read fractional digits as an integer number of nanoseconds
ExessResult
read_nanoseconds(uint32_t* out, const char* str);

/// Write nanoseconds as fractional digits
size_t
write_nanoseconds(uint32_t nanosecond, size_t buf_size, char* buf, size_t i);

/// Write a complete time with timezone suffix if necessary
ExessResult
write_time(ExessTime value, size_t buf_size, char* buf, size_t offset);

#endif // EXESS_TIME_UTILS_H
