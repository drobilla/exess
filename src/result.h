// Copyright 2019-2021 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#ifndef EXESS_RESULT_H
#define EXESS_RESULT_H

#include "exess/exess.h"

#include <stddef.h>

static inline ExessResult
result(const ExessStatus status, const size_t count)
{
  const ExessResult r = {status, count};
  return r;
}

#endif // EXESS_RESULT_H
