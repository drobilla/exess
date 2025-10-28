// Copyright 2019-2021 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#ifndef EXESS_SRC_STRTOD_H
#define EXESS_SRC_STRTOD_H

#include "floating_decimal.h"

#include <exess/exess.h>

ExessResult
parse_decimal(const char* str, ExessFloatingDecimal* out);

ExessResult
parse_double(const char* str, ExessFloatingDecimal* out);

double
decimal_to_double(ExessFloatingDecimal in);

#endif // EXESS_SRC_STRTOD_H
