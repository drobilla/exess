// Copyright 2019-2026 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#ifndef EXESS_SRC_NUMBER_UTILS_H
#define EXESS_SRC_NUMBER_UTILS_H

#include <exess/exess.h>

#include <stddef.h>
#include <stdint.h>

EXESS_NONBLOCKING size_t
read_sign(int* sign, const char* str);

EXESS_NONBLOCKING ExessResult
read_two_digit_number(uint8_t*    out,
                      uint8_t     min_value,
                      uint8_t     max_value,
                      const char* str);

EXESS_NONBLOCKING size_t
write_two_digit_number(uint8_t value, size_t buf_size, char* buf, size_t i);

EXESS_NONBLOCKING size_t
write_zeros(size_t n, size_t buf_size, char* buf, size_t i);

EXESS_NONBLOCKING ExessResult
read_digits(uint64_t* out, const char* str);

EXESS_NONBLOCKING ExessResult
write_digits(uint64_t value, size_t buf_size, char* buf, size_t i);

#endif // EXESS_SRC_NUMBER_UTILS_H
