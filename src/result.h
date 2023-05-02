// Copyright 2019-2021 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#ifndef EXESS_SRC_RESULT_H
#define EXESS_SRC_RESULT_H

#include "exess/exess.h"

#include <stddef.h>

static inline ExessResult
result(const ExessStatus status, const size_t count)
{
  const ExessResult r = {status, count};
  return r;
}

static inline ExessVariableResult
vresult(const ExessStatus status,
        const size_t      read_count,
        const size_t      write_count)
{
  const ExessVariableResult r = {status, read_count, write_count};
  return r;
}

#endif // EXESS_SRC_RESULT_H
