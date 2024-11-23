// Copyright 2019-2023 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#ifndef EXESS_SRC_YEAR_H
#define EXESS_SRC_YEAR_H

#include <exess/exess.h>

#include <stddef.h>
#include <stdint.h>

ExessResult
read_year_number(int16_t* out, const char* str);

ExessResult
write_year_number(int16_t value, size_t buf_size, char* buf);

#endif // EXESS_SRC_YEAR_H
