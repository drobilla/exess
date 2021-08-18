// Copyright 2021 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#ifndef EXESS_TIMEZONE_H
#define EXESS_TIMEZONE_H

#include "exess/exess.h"

#include <stddef.h>

/// The maximum length of a canonical timezone string, 6
#define EXESS_MAX_TIMEZONE_LENGTH 6

/**
   Read a timezone string after any leading whitespace.

   @param out Set to the parsed value, or false on error.
   @param str String input.
   @return The `count` of characters read, and a `status` code.
*/
EXESS_API
ExessResult
exess_read_timezone(ExessTimezone* EXESS_NONNULL out,
                    const char* EXESS_NONNULL    str);

/**
   Write a canonical timezone suffix.

   The output is always in canonical form, either `Z` for UTC or a signed hour
   and minute offset with leading zeros, like `-05:30` or `+14:00`.

   @param value Value to write.
   @param buf_size The size of `buf` in bytes.
   @param buf Output buffer, or null to only measure.
   @param o The current write offset in `buf`

   @return #EXESS_SUCCESS on success, #EXESS_NO_SPACE if the buffer is too
   small, or #EXESS_BAD_VALUE if the value is invalid.
*/
ExessResult
write_timezone(ExessTimezone        value,
               size_t               buf_size,
               char* EXESS_NULLABLE buf,
               size_t               o);

#endif // EXESS_TIMEZONE_H
