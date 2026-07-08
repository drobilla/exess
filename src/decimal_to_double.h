// Copyright 2019-2026 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#ifndef EXESS_SRC_DECIMAL_TO_DOUBLE_H
#define EXESS_SRC_DECIMAL_TO_DOUBLE_H

#include "attributes.h"
#include "big_decimal.h"

EXESS_I_NONBLOCKING double
decimal_to_double(BigDecimal in, const char* digits);

#endif // EXESS_SRC_DECIMAL_TO_DOUBLE_H
