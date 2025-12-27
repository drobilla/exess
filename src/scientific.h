// Copyright 2019-2025 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#ifndef EXESS_SRC_SCIENTIFIC_H
#define EXESS_SRC_SCIENTIFIC_H

#include "big_decimal.h"

#include <exess/exess.h>

#include <stddef.h>

ExessResult
write_scientific(BigDecimal  value,
                 const char* digits,
                 size_t      buf_size,
                 char*       buf);

#endif // EXESS_SRC_SCIENTIFIC_H
