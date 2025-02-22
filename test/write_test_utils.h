// Copyright 2025 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#ifndef EXESS_TEST_WRITE_TEST_UTILS_H
#define EXESS_TEST_WRITE_TEST_UTILS_H

#include <stddef.h>

static inline void
init_out_buf(const size_t buf_size, char* const buf)
{
  for (size_t i = 0; i < buf_size; ++i) {
    buf[i] = (char)(i + 1);
  }
}

#endif // EXESS_TEST_WRITE_TEST_UTILS_H
