// Copyright 2019-2025 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#ifndef EXESS_SRC_READ_UTILS_H
#define EXESS_SRC_READ_UTILS_H

#include <exess/exess.h>

#include <stddef.h>
#include <stdint.h>

EXESS_PURE_FUNC size_t
skip_whitespace(const char* str);

ExessResult
read_two_digit_number(uint8_t*    out,
                      uint8_t     min_value,
                      uint8_t     max_value,
                      const char* str);

size_t
read_sign(int* sign, const char* str);

ExessResult
read_digits(uint64_t* out, const char* str);

#endif // EXESS_SRC_READ_UTILS_H
