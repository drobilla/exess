// Copyright 2019-2021 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#ifndef EXESS_SRC_STRTOD_H
#define EXESS_SRC_STRTOD_H

#include "decimal.h"

#include "exess/exess.h"

ExessResult
parse_decimal(ExessDecimalDouble* out, const char* str);

ExessResult
parse_double(ExessDecimalDouble* out, const char* str);

double
parsed_double_to_double(ExessDecimalDouble in);

#endif // EXESS_SRC_STRTOD_H
