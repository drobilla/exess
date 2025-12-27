// Copyright 2019-2025 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#ifndef EXESS_SRC_PARSE_DECIMAL_H
#define EXESS_SRC_PARSE_DECIMAL_H

#include "big_decimal.h"

#include <exess/exess.h>

#include <stddef.h>

ExessResult
parse_decimal(const char* str,
              BigDecimal* out,
              size_t      digits_size,
              char*       digits);

#endif // EXESS_SRC_PARSE_DECIMAL_H
