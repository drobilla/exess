// Copyright 2019-2025 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#ifndef EXESS_TEST_INTEGER_TEST_UTILS_H
#define EXESS_TEST_INTEGER_TEST_UTILS_H

#include <exess/exess.h>

#include <assert.h>
#include <stddef.h>

static inline size_t
set_integer_value(const size_t  value_size,
                  void* const   value,
                  const int64_t number)
{
  const ExessResult r = exess_coerce_value(
    0U, EXESS_LONG, sizeof(number), &number, EXESS_INTEGER, value_size, value);

  assert(!r.status);
  return r.count;
}

#endif // EXESS_TEST_INTEGER_TEST_UTILS_H
