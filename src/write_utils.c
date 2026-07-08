// Copyright 2019-2026 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#include "write_utils.h"

#include "result.h"

#include <exess/exess.h>

#include <stdbool.h>
#include <string.h>

EXESS_NONBLOCKING size_t
write_char(const char c, const size_t buf_size, char* const buf, const size_t i)
{
  if (buf && i < buf_size) {
    buf[i] = c;
  }

  return 1;
}

EXESS_NONBLOCKING size_t
write_string(const size_t      len,
             const char* const str,
             const size_t      buf_size,
             char* const       buf,
             const size_t      i)
{
  if (buf && i + len <= buf_size) {
    memcpy(buf + i, str, len);
  }

  return len;
}

EXESS_NONBLOCKING ExessResult
end_write(const ExessStatus status,
          const size_t      buf_size,
          char* const       buf,
          const size_t      i)
{
  const bool overflow =
    (status == EXESS_NO_SPACE || (!status && i >= buf_size));

  // Truncate on error and null terminate output if at all possible
  const size_t end = (status || overflow) ? 0 : i;
  if (buf && end < buf_size) {
    buf[end] = '\0';
  }

  return overflow ? RESULT(EXESS_NO_SPACE, i) : RESULT(status, end);
}

EXESS_NONBLOCKING ExessResult
write_special(const size_t      string_length,
              const char* const string,
              const size_t      buf_size,
              char* const       buf)
{
  if (string_length >= buf_size) {
    return end_write(EXESS_NO_SPACE, buf_size, buf, string_length);
  }

  memcpy(buf, string, string_length);
  return end_write(EXESS_SUCCESS, buf_size, buf, string_length);
}
