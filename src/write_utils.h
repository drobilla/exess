// Copyright 2019-2021 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#ifndef EXESS_SRC_WRITE_UTILS_H
#define EXESS_SRC_WRITE_UTILS_H

#include <exess/exess.h>

#include <stddef.h>
#include <stdint.h>

size_t
write_char(char c, size_t buf_size, char* buf, size_t i);

size_t
write_string(size_t len, const char* str, size_t buf_size, char* buf, size_t i);

ExessResult
end_write(ExessStatus status, size_t buf_size, char* buf, size_t i);

ExessResult
write_digits(uint64_t value, size_t buf_size, char* buf, size_t i);

size_t
write_two_digit_number(uint8_t value, size_t buf_size, char* buf, size_t i);

ExessResult
write_special(size_t      string_length,
              const char* string,
              size_t      buf_size,
              char*       buf);

#endif // EXESS_SRC_WRITE_UTILS_H
