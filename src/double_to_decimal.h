// Copyright 2019-2025 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#ifndef EXESS_SRC_DOUBLE_TO_DECIMAL_H
#define EXESS_SRC_DOUBLE_TO_DECIMAL_H

#include "big_decimal.h"

#include <stddef.h>

BigDecimal
double_to_decimal(double d, size_t digits_size, char* digits);

#endif // EXESS_SRC_DOUBLE_TO_DECIMAL_H
