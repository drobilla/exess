// Copyright 2019-2021 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#ifndef EXESS_SRC_SCIENTIFIC_H
#define EXESS_SRC_SCIENTIFIC_H

#include "big_decimal.h"

#include <exess/exess.h>

#include <stddef.h>

ExessResult
write_scientific(BigDecimal value, size_t buf_size, char* EXESS_NULLABLE buf);

#endif // EXESS_SRC_SCIENTIFIC_H
