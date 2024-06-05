// Copyright 2011-2021 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#include "macros.h"
#include "read_utils.h"
#include "result.h"
#include "string_utils.h"
#include "write_utils.h"

#include "exess/exess.h"

#include <assert.h>
#include <stdint.h>
#include <string.h>

// Map a 6-bit base64 group to a base64 digit
EXESS_PURE_FUNC static inline uint8_t
map(const unsigned group)
{
  assert(group < 64);

  // See <http://tools.ietf.org/html/rfc3548#section-3>.
  static const uint8_t b64_map[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

  return b64_map[group];
}

// Unmap a base64 digit to the numeric value used for decoding
static inline uint8_t
unmap(const uint8_t in)
{
  /* Table indexed by encoded characters that contains the numeric value used
     for decoding, shifted up by 47 to be in the range of printable ASCII.  A
     '$' is a placeholder for characters not in the base64 alphabet. */
  static const uint8_t b64_unmap[] =
    "$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$m$$$ncdefghijkl$$$$$$"
    "$/0123456789:;<=>?@ABCDEFGH$$$$$$IJKLMNOPQRSTUVWXYZ[\\]^_`ab$$$$"
    "$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$"
    "$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$";

  return (uint8_t)(b64_unmap[in] - 47U);
}

static char
next_char(const char* const str, size_t* const i)
{
  *i += skip_whitespace(str + *i);

  return str[*i];
}

size_t
exess_base64_decoded_size(const size_t length)
{
  return (length * 3) / 4 + 2;
}

ExessVariableResult
exess_read_base64(const size_t out_size, void* const out, const char* const str)
{
  uint8_t* const       uout = (uint8_t*)out;
  const uint8_t* const ustr = (const uint8_t*)str;
  size_t               i    = 0U;
  size_t               o    = 0U;

  while (str[i]) {
    // Skip leading whitespace
    i += skip_whitespace(str + i);
    if (!str[i]) {
      break;
    }

    // Read next chunk of 4 input characters
    uint8_t in[] = {'=', '=', '=', '='};
    for (size_t j = 0; j < 4; ++j) {
      const char c = next_char(str, &i);
      if (!is_base64(c)) {
        return vresult(EXESS_EXPECTED_BASE64, i, o);
      }

      in[j] = ustr[i++];
    }

    if (in[0] == '=' || in[1] == '=' || (in[2] == '=' && in[3] != '=')) {
      return vresult(EXESS_BAD_VALUE, i, o);
    }

    const size_t n_bytes = 1U + (in[2] != '=') + (in[3] != '=');
    if (o + n_bytes > out_size) {
      return vresult(EXESS_NO_SPACE, i, o);
    }

    const uint8_t a1 = (uint8_t)(unmap(in[0]) << 2U);
    const uint8_t a2 = unmap(in[1]) >> 4U;

    uout[o++] = a1 | a2;

    if (in[2] != '=') {
      const uint8_t b1 = (uint8_t)(((unsigned)unmap(in[1]) << 4U) & 0xF0U);
      const uint8_t b2 = unmap(in[2]) >> 2U;

      uout[o++] = b1 | b2;
    }

    if (in[3] != '=') {
      const uint8_t c1 = (uint8_t)(((unsigned)unmap(in[2]) << 6U) & 0xC0U);
      const uint8_t c2 = unmap(in[3]);

      uout[o++] = c1 | c2;
    }
  }

  return vresult(EXESS_SUCCESS, i, o);
}

ExessResult
exess_write_base64(const size_t      data_size,
                   const void* const data,
                   const size_t      buf_size,
                   char* const       buf)
{
  const size_t length = (data_size + 2) / 3 * 4;
  if (!buf) {
    return result(EXESS_SUCCESS, length);
  }

  if (buf_size < length + 1) {
    return result(EXESS_NO_SPACE, 0);
  }

  uint8_t* const out = (uint8_t*)buf;

  size_t o = 0;
  for (size_t i = 0; i < data_size; i += 3, o += 4) {
    uint8_t      in[4] = {0, 0, 0, 0};
    const size_t n_in  = MIN(3, data_size - i);
    memcpy(in, (const uint8_t*)data + i, n_in);

    out[o]     = map(in[0] >> 2U);
    out[o + 1] = map(((in[0] & 0x03U) << 4U) | ((in[1] & 0xF0U) >> 4U));
    out[o + 2] =
      ((n_in > 1U) ? map(((in[1] & 0x0FU) << 2U) | ((in[2] & 0xC0U) >> 6U))
                   : '=');

    out[o + 3] = ((n_in > 2U) ? map(in[2] & 0x3FU) : '=');
  }

  return end_write(EXESS_SUCCESS, buf_size, buf, o);
}
