// Copyright 2019-2026 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#ifndef EXESS_SRC_TIME_UTILS_H
#define EXESS_SRC_TIME_UTILS_H

#include <exess/exess.h>

#include <stddef.h>
#include <stdint.h>

/// Read fractional digits as an integer number of nanoseconds
EXESS_NONBLOCKING ExessResult
read_nanoseconds(uint32_t* out, const char* str);

/// Write nanoseconds as fractional digits
EXESS_NONBLOCKING size_t
write_nanoseconds(uint32_t nanosecond, size_t buf_size, char* buf, size_t i);

/// Read a time
EXESS_NONBLOCKING ExessResult
read_time(ExessTime* out, const char* str);

/// Write a complete time with timezone suffix if necessary
EXESS_NONBLOCKING ExessResult
write_time(ExessTime value, size_t buf_size, char* buf, ExessResult r);

#endif // EXESS_SRC_TIME_UTILS_H
