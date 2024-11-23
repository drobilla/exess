// Copyright 2019-2021 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#include "read_utils.h"
#include "write_utils.h"

#include <exess/exess.h>

#include <stdbool.h>
#include <string.h>

ExessResult
exess_read_boolean(bool* const out, const char* const str)
{
  size_t      i = skip_whitespace(str);
  ExessResult r = {EXESS_EXPECTED_BOOLEAN, i};

  *out = false;

  switch (str[i]) {
  case '0':
    return end_read(EXESS_SUCCESS, str, i + 1);

  case '1':
    *out = true;
    return end_read(EXESS_SUCCESS, str, i + 1);

  case 't':
    if (!strncmp(str + i, "true", 4)) {
      *out = true;
      return end_read(EXESS_SUCCESS, str, i + 4U);
    }
    break;

  case 'f':
    if (!strncmp(str + i, "false", 5)) {
      return end_read(EXESS_SUCCESS, str, i + 5U);
    }
    break;

  default:
    break;
  }

  return end_read(r.status, str, r.count);
}

ExessResult
exess_write_boolean(const bool value, const size_t buf_size, char* const buf)
{
  return end_write(EXESS_SUCCESS,
                   buf_size,
                   buf,
                   value ? write_string(4, "true", buf_size, buf, 0)
                         : write_string(5, "false", buf_size, buf, 0));
}
