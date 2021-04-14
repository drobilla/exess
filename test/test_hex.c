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

#undef NDEBUG

#include "exess/exess.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static void
check_read(const char* const string,
           const ExessStatus expected_status,
           const size_t      expected_value_length,
           const char* const expected_value,
           const size_t      expected_value_size,
           const size_t      expected_count)
{
  char      buf[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
  ExessBlob blob   = {sizeof(buf), buf};

  ExessResult r = exess_read_hex(&blob, string);
  assert(r.status == expected_status);
  assert(r.count == expected_count);
  assert(r.status || blob.size == expected_value_size);
  if (expected_value_length > 0) {
    assert(!strncmp(buf, expected_value, expected_value_length));
    assert(blob.size <= exess_hex_decoded_size(strlen(string)));
  }
}

static void
test_lowercase(void)
{
  char      buf[6] = {0, 0, 0, 0, 0, 0};
  ExessBlob blob   = {sizeof(buf), buf};

  ExessResult r = exess_read_hex(&blob, "6A6B6C6D6E6F");
  assert(r.status == EXESS_SUCCESS);
  assert(r.count == 12);
  assert(blob.size == 6);
  assert(!strncmp((const char*)blob.data, "jklmno", 6));

  r = exess_read_hex(&blob, "6a6b6c6d6e6f");
  assert(r.status == EXESS_SUCCESS);
  assert(r.count == 12);
  assert(blob.size == 6);
  assert(!strncmp((const char*)blob.data, "jklmno", 6));
}

static void
test_whitespace(void)
{
  check_read("666F6F", EXESS_SUCCESS, 3, "foo", 3, 6);
  check_read(" 666F6F", EXESS_SUCCESS, 3, "foo", 3, 7);
  check_read("6\f66F6F", EXESS_SUCCESS, 3, "foo", 3, 7);
  check_read("66\n6F6F", EXESS_SUCCESS, 3, "foo", 3, 7);
  check_read("666\rF6F", EXESS_SUCCESS, 3, "foo", 3, 7);
  check_read("666F\t6F", EXESS_SUCCESS, 3, "foo", 3, 7);
  check_read(" \f\n\r\t\v666F6F", EXESS_SUCCESS, 3, "foo", 3, 12);
  check_read("666F6F \f\n\r\t\v", EXESS_SUCCESS, 3, "foo", 3, 12);
}

static void
test_syntax_errors(void)
{
  check_read("G6", EXESS_EXPECTED_HEX, 0, NULL, 0, 1);
  check_read("g6", EXESS_EXPECTED_HEX, 0, NULL, 0, 1);
  check_read("!6", EXESS_EXPECTED_HEX, 0, NULL, 0, 1);
  check_read("^6", EXESS_EXPECTED_HEX, 0, NULL, 0, 1);
  check_read("6G", EXESS_EXPECTED_HEX, 0, NULL, 0, 2);
  check_read("6g", EXESS_EXPECTED_HEX, 0, NULL, 0, 2);
  check_read("6!", EXESS_EXPECTED_HEX, 0, NULL, 0, 2);
  check_read("6^", EXESS_EXPECTED_HEX, 0, NULL, 0, 2);
  check_read("6", EXESS_EXPECTED_HEX, 0, NULL, 0, 1);
  check_read("66G6", EXESS_EXPECTED_HEX, 0, NULL, 1, 3);
  check_read("66 G6", EXESS_EXPECTED_HEX, 0, NULL, 1, 4);
}

static void
test_read_overflow(void)
{
  char      buf[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
  ExessBlob blob0  = {0, buf};
  ExessBlob blob1  = {1, buf};
  ExessBlob blob2  = {2, buf};

  ExessResult r = exess_read_hex(&blob0, "666F6F");
  assert(r.status == EXESS_NO_SPACE);
  assert(r.count == 2);
  assert(blob0.size == 0);

  r = exess_read_hex(&blob1, "666F6F");
  assert(r.status == EXESS_NO_SPACE);
  assert(r.count == 4);
  assert(blob1.size == 1);

  r = exess_read_hex(&blob2, "666F6F");
  assert(r.status == EXESS_NO_SPACE);
  assert(r.count == 6);
  assert(blob2.size == 2);
}

static void
test_write_overflow(void)
{
  char buf[7] = {1, 2, 3, 4, 5, 6, 7};

  assert(exess_write_hex(3, "foo", 0, buf).status == EXESS_NO_SPACE);
  assert(exess_write_hex(3, "foo", 1, buf).status == EXESS_NO_SPACE);
  assert(exess_write_hex(3, "foo", 2, buf).status == EXESS_NO_SPACE);
  assert(exess_write_hex(3, "foo", 3, buf).status == EXESS_NO_SPACE);
  assert(exess_write_hex(3, "foo", 4, buf).status == EXESS_NO_SPACE);
  assert(exess_write_hex(3, "foo", 5, buf).status == EXESS_NO_SPACE);
  assert(exess_write_hex(3, "foo", 6, buf).status == EXESS_NO_SPACE);
  assert(exess_write_hex(3, "foo", 7, buf).status == EXESS_SUCCESS);
}

static void
test_round_trip(void)
{
  for (size_t size = 1; size < 256; ++size) {
    // Allocate and generate data
    uint8_t* const data = (uint8_t*)malloc(size);
    for (size_t i = 0; i < size; ++i) {
      data[i] = (uint8_t)((size + i) % 256);
    }

    // Allocate buffer for encoding with minimum required size
    const size_t str_len = exess_write_hex(size, data, 0, NULL).count;
    char* const  str     = (char*)malloc(str_len + 1);

    // Encode data to string buffer
    assert(!exess_write_hex(size, data, str_len + 1, str).status);
    assert(strlen(str) == str_len);
    assert(str_len % 2 == 0);

    // Allocate buffer for decoded data with the same size as the input
    uint8_t* const decoded      = (uint8_t*)malloc(size);
    ExessBlob      decoded_blob = {size, decoded};

    // Decode and check that data matches the original input
    assert(!exess_read_hex(&decoded_blob, str).status);
    assert(decoded_blob.size == size);
    assert(!memcmp(decoded, data, size));

    free(decoded);
    free(str);
    free(data);
  }
}

int
main(void)
{
  test_lowercase();
  test_whitespace();
  test_syntax_errors();
  test_read_overflow();
  test_write_overflow();
  test_round_trip();

  return 0;
}
