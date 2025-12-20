// Copyright 2011-2025 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#include "char_utils.h"
#include "read_utils.h"
#include "result.h"
#include "write_utils.h"

#include <exess/exess.h>

#include <stdint.h>
#include <string.h>

/// Hex encoding table
static const char hex_map[] = "0123456789ABCDEF";

static uint8_t
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

size_t
exess_decoded_hex_size(const size_t length)
{
  return length / 2;
}

ExessVariableResult
exess_read_hex(const char* const str, const size_t out_size, void* const out)
{
  uint8_t* const uout  = (uint8_t*)out;
  const size_t   first = skip_whitespace(str);
  size_t         i     = first;
  size_t         o     = 0U;

  while (str[i]) {
    const char    hi_char = str[i];
    const uint8_t hi      = decode_nibble(hi_char);
    if (hi == UINT8_MAX) {
      return VRESULT(i == first ? EXESS_EXPECTED_HEX : EXESS_SUCCESS, i, o);
    }

    ++i;

    const char lo_char = str[i];
    if (!lo_char) {
      return VRESULT(EXESS_EXPECTED_HEX, i, o);
    }

    const uint8_t lo = decode_nibble(lo_char);
    if (lo == UINT8_MAX) {
      return VRESULT(EXESS_EXPECTED_HEX, i, o);
    }

    ++i;

    if (o < out_size) {
      uout[o] = (uint8_t)(((unsigned)hi << 4U) | lo);
    }

    ++o;
  }

  return VRESULT(o <= out_size ? EXESS_SUCCESS : EXESS_NO_SPACE, i, o);
}

ExessResult
exess_write_hex(const size_t      data_size,
                const void* const data,
                const size_t      buf_size,
                char* const       buf)
{
  const size_t length = 2 * data_size;
  if (!buf || length >= buf_size) {
    return end_write(EXESS_NO_SPACE, buf_size, buf, length);
  }

  const uint8_t* const in = (const uint8_t*)data;
  size_t               o  = 0U;

  for (size_t i = 0; i < data_size; ++i) {
    const uint8_t hi = (in[i] & 0xF0U) >> 4U;
    const uint8_t lo = (in[i] & 0x0FU);

    buf[o++] = hex_map[hi];
    buf[o++] = hex_map[lo];
  }

  return end_write(EXESS_SUCCESS, buf_size, buf, o);
}
