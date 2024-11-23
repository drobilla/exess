// Copyright 2019-2021 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#ifndef EXESS_SRC_READ_UTILS_H
#define EXESS_SRC_READ_UTILS_H

#include <exess/exess.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

EXESS_PURE_FUNC size_t
skip_whitespace(const char* str);

EXESS_CONST_FUNC bool
is_end(char c);

ExessResult
read_two_digit_number(uint8_t*    out,
                      uint8_t     min_value,
                      uint8_t     max_value,
                      const char* str);

EXESS_PURE_FUNC ExessResult
end_read(ExessStatus status, const char* str, size_t i);

#endif // EXESS_SRC_READ_UTILS_H
