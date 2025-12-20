// Copyright 2019-2025 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#ifndef EXESS_SRC_DOUBLE_TO_DECIMAL_H
#define EXESS_SRC_DOUBLE_TO_DECIMAL_H

#include "big_decimal.h"

BigDecimal
double_to_decimal(double d, unsigned max_precision);

#endif // EXESS_SRC_DOUBLE_TO_DECIMAL_H
