/*
  Copyright 2011-2021 David Robillard <d@drobilla.net>

  Permission to use, copy, modify, and/or distribute this software for any
  purpose with or without fee is hereby granted, provided that the above
  copyright notice and this permission notice appear in all copies.

  THIS SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
  WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
  MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
  ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
  WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
  ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
  OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/

#ifndef EXESS_STRING_UTILS_H
#define EXESS_STRING_UTILS_H

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
  switch (c) {
  case ' ':
  case '\f':
  case '\n':
  case '\r':
  case '\t':
  case '\v':
    return true;
  default:
    return false;
  }
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

#endif // EXESS_STRING_UTILS_H
