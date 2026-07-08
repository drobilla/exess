// Copyright 2026 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#ifndef EXESS_SRC_FIXED_H
#define EXESS_SRC_FIXED_H

#include <exess/exess.h>

#include <stdint.h>

EXESS_NONBLOCKING ExessResult
read_signed(const char* str, int64_t min, int64_t max, int64_t* out);

EXESS_NONBLOCKING ExessResult
read_unsigned(const char* str, uint64_t max, uint64_t* out);

#endif // EXESS_SRC_FIXED_H
