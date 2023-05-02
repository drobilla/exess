// Copyright 2011-2021 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#ifndef EXESS_SRC_STRING_UTILS_H
#define EXESS_SRC_STRING_UTILS_H

#include <stdbool.h>

/// Return true if `c` lies within [`min`...`max`] (inclusive)
static inline bool
in_range(const int c, const int min, const int max)
{
  return (c >= min && c <= max);
}

/// Return true if `c` is a whitespace character
static inline bool
is_space(const int c)
{
  return c == ' ' || c == '\f' || c == '\n' || c == '\r' || c == '\t' ||
         c == '\v';
}

/// ALPHA ::= [A-Za-z]
static inline bool
is_alpha(const int c)
{
  return in_range(c, 'A', 'Z') || in_range(c, 'a', 'z');
}

/// DIGIT ::= [0-9]
static inline bool
is_digit(const int c)
{
  return in_range(c, '0', '9');
}

/// HEXDIG ::= DIGIT | "A" | "B" | "C" | "D" | "E" | "F"
static inline bool
is_hexdig(const int c)
{
  return is_digit(c) || in_range(c, 'A', 'F');
}

/// BASE64 ::= ALPHA | DIGIT | "+" | "/" | "="
static inline bool
is_base64(const int c)
{
  return is_alpha(c) || is_digit(c) || c == '+' || c == '/' || c == '=';
}

#endif // EXESS_SRC_STRING_UTILS_H
