// Copyright 2019-2021 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#ifndef EXESS_SRC_SCIENTIFIC_H
#define EXESS_SRC_SCIENTIFIC_H

#include "floating_decimal.h"

#include <exess/exess.h>

#include <stddef.h>

EXESS_CONST_FUNC size_t
scientific_string_length(ExessFloatingDecimal value);

ExessResult
write_scientific(ExessFloatingDecimal value, size_t n, char* EXESS_NONNULL buf);

#endif // EXESS_SRC_SCIENTIFIC_H
