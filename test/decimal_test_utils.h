// Copyright 2019-2025 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#ifndef EXESS_TEST_DECIMAL_TEST_UTILS_H
#define EXESS_TEST_DECIMAL_TEST_UTILS_H

#include <exess/exess.h>

#include <assert.h>
#include <stddef.h>

static inline size_t
set_decimal_value(const size_t value_size,
                  void* const  value,
                  const double number)
{
  const ExessResult r = exess_coerce_value(0U,
                                           EXESS_DOUBLE,
                                           sizeof(number),
                                           &number,
                                           EXESS_DECIMAL,
                                           value_size,
                                           value);
  assert(!r.status);
  return r.count;
}

#endif // EXESS_TEST_DECIMAL_TEST_UTILS_H
