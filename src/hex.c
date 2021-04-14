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

#include "read_utils.h"
#include "string_utils.h"
#include "write_utils.h"

#include "exess/exess.h"

#include <stdint.h>
#include <string.h>

/// Hex encoding table
static const char hex_map[] = "0123456789ABCDEF";

static inline uint8_t
decode_nibble(const char c)
{
  if (is_digit(c)) {
    return (uint8_t)(c - '0');
  }

  if (c >= 'A' && c <= 'F') {
    return (uint8_t)(10 + c - 'A');
  }

  if (c >= 'a' && c <= 'f') {
    return (uint8_t)(10 + c - 'a');
  }

  return UINT8_MAX;
}

static char
next_char(const char* const str, size_t* const i)
{
  *i += skip_whitespace(str + *i);

  return str[*i];
}

size_t
exess_hex_decoded_size(const size_t length)
{
  return length / 2;
}

ExessResult
exess_read_hex(ExessBlob* const out, const char* const str)
{
  uint8_t* const uout = (uint8_t*)out->data;
  size_t         i    = 0u;
  size_t         o    = 0u;

  while (str[i]) {
    const char hi_char = next_char(str, &i);
    if (!hi_char) {
      break;
    }

    ++i;

    const uint8_t hi = decode_nibble(hi_char);
    if (hi == UINT8_MAX) {
      return result(EXESS_EXPECTED_HEX, i);
    }

    const char lo_char = next_char(str, &i);
    if (!lo_char) {
      return result(EXESS_EXPECTED_HEX, i);
    }

    ++i;

    const uint8_t lo = decode_nibble(lo_char);
    if (lo == UINT8_MAX) {
      return result(EXESS_EXPECTED_HEX, i);
    }

    if (o >= out->size) {
      return result(EXESS_NO_SPACE, i);
    }

    uout[o++] = (uint8_t)(((unsigned)hi << 4u) | lo);
  }

  out->size = o;
  return result(EXESS_SUCCESS, i);
}

ExessResult
exess_write_hex(const size_t      data_size,
                const void* const data,
                const size_t      buf_size,
                char* const       buf)
{
  const size_t length = 2 * data_size;
  if (!buf) {
    return result(EXESS_SUCCESS, length);
  }

  if (buf_size < length + 1) {
    return result(EXESS_NO_SPACE, 0);
  }

  const uint8_t* const in = (const uint8_t*)data;
  size_t               o  = 0u;

  for (size_t i = 0; i < data_size; ++i) {
    const uint8_t hi = (in[i] & 0xF0u) >> 4u;
    const uint8_t lo = (in[i] & 0x0Fu);

    buf[o++] = hex_map[hi];
    buf[o++] = hex_map[lo];
  }

  return end_write(EXESS_SUCCESS, buf_size, buf, o);
}
