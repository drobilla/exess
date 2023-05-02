// Copyright 2011-2021 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#include "read_utils.h"
#include "result.h"
#include "string_utils.h"
#include "write_utils.h"

#include "exess/exess.h"

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

ExessVariableResult
exess_read_hex(const size_t out_size, void* const out, const char* const str)
{
  uint8_t* const uout = (uint8_t*)out;
  size_t         i    = 0U;
  size_t         o    = 0U;

  while (str[i]) {
    const char hi_char = next_char(str, &i);
    if (!hi_char) {
      break;
    }

    ++i;

    const uint8_t hi = decode_nibble(hi_char);
    if (hi == UINT8_MAX) {
      return vresult(EXESS_EXPECTED_HEX, i, o);
    }

    const char lo_char = next_char(str, &i);
    if (!lo_char) {
      return vresult(EXESS_EXPECTED_HEX, i, o);
    }

    ++i;

    const uint8_t lo = decode_nibble(lo_char);
    if (lo == UINT8_MAX) {
      return vresult(EXESS_EXPECTED_HEX, i, o);
    }

    if (o >= out_size) {
      return vresult(EXESS_NO_SPACE, i, o);
    }

    uout[o++] = (uint8_t)(((unsigned)hi << 4U) | lo);
  }

  return vresult(EXESS_SUCCESS, i, o);
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
  size_t               o  = 0U;

  for (size_t i = 0; i < data_size; ++i) {
    const uint8_t hi = (in[i] & 0xF0U) >> 4U;
    const uint8_t lo = (in[i] & 0x0FU);

    buf[o++] = hex_map[hi];
    buf[o++] = hex_map[lo];
  }

  return end_write(EXESS_SUCCESS, buf_size, buf, o);
}
