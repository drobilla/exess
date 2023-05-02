// Copyright 2019-2021 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#ifndef EXESS_SRC_WRITE_UTILS_H
#define EXESS_SRC_WRITE_UTILS_H

#include "exess/exess.h"

#include <stddef.h>
#include <stdint.h>
#include <string.h>

static inline size_t
write_char(const char c, size_t buf_size, char* const buf, const size_t i)
{
  if (buf && buf_size >= i + 1) {
    buf[i] = c;
  }

  return 1;
}

static inline size_t
write_string(const size_t len,
             const char*  str,
             const size_t buf_size,
             char* const  buf,
             const size_t i)
{
  if (buf && buf_size >= i + len + 1) {
    memcpy(buf + i, str, len);
    buf[i + len] = 0;
  }

  return len;
}

static inline ExessResult
end_write(const ExessStatus status,
          const size_t      buf_size,
          char* const       buf,
          const size_t      i)
{
  ExessResult r = {status, status > EXESS_EXPECTED_END ? 0 : i};

  if (buf) {
    if (!status && i >= buf_size) {
      r.status = EXESS_NO_SPACE;
      r.count  = 0;
    }

    if (r.count < buf_size) {
      buf[r.count] = '\0';
    }
  }

  return r;
}

ExessResult
write_digits(uint64_t value, size_t buf_size, char* buf, size_t i);

size_t
write_two_digit_number(uint8_t value, size_t buf_size, char* buf, size_t i);

ExessResult
write_special(size_t      string_length,
              const char* string,
              size_t      buf_size,
              char*       buf);

#endif // EXESS_SRC_WRITE_UTILS_H
