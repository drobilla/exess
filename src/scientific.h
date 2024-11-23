// Copyright 2019-2021 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#ifndef EXESS_SRC_SCIENTIFIC_H
#define EXESS_SRC_SCIENTIFIC_H

#include "decimal.h"

#include <exess/exess.h>

#include <stddef.h>

EXESS_CONST_FUNC size_t
exess_scientific_string_length(ExessDecimalDouble value);

ExessResult
exess_write_scientific(ExessDecimalDouble  value,
                       size_t              n,
                       char* EXESS_NONNULL buf);

#endif // EXESS_SRC_SCIENTIFIC_H
