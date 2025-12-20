// Copyright 2019-2021 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#ifndef EXESS_SRC_PARSE_DECIMAL_H
#define EXESS_SRC_PARSE_DECIMAL_H

#include "big_decimal.h"

#include <exess/exess.h>

ExessResult
parse_decimal(const char* str, BigDecimal* out);

#endif // EXESS_SRC_PARSE_DECIMAL_H
