// Copyright 2019-2026 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#include "read_utils.h"

#include "char_utils.h"

#include <exess/exess.h>

EXESS_NONBLOCKING size_t
skip_whitespace(const char* const str)
{
  size_t i = 0;
  while (is_space(str[i])) {
    ++i;
  }

  return i;
}
