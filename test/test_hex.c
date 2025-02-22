// Copyright 2011-2025 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#undef NDEBUG

#include <exess/exess.h>

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
  char buf[9] = {1, 2, 3, 4, 5, 6, 7, 8, 9};

  ExessVariableResult r = exess_read_hex(sizeof(buf), buf, string);
  assert(r.status == expected_status);
  assert(r.read_count == expected_count);
  assert(r.status || r.write_count == expected_value_size);
  assert(r.write_count > 0 || buf[0] == 1);
  if (expected_value_length > 0) {
    assert(!strncmp(buf, expected_value, expected_value_length));
    assert(r.write_count <= exess_decoded_hex_size(strlen(string)));
  }
}

static void
test_lowercase(void)
{
  char buf[6] = {1, 2, 3, 4, 5, 6};

  ExessVariableResult r = exess_read_hex(sizeof(buf), buf, "6A6B6C6D6E6F");
  assert(r.status == EXESS_SUCCESS);
  assert(r.read_count == 12);
  assert(r.write_count == 6);
  assert(!strncmp(buf, "jklmno", 6));

  r = exess_read_hex(sizeof(buf), buf, "6a6b6c6d6e6f");
  assert(r.status == EXESS_SUCCESS);
  assert(r.read_count == 12);
  assert(r.write_count == 6);
  assert(!strncmp(buf, "jklmno", 6));
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
  char buf[9] = {1, 2, 3, 4, 5, 6, 7, 8, 9};

  ExessVariableResult r = exess_read_hex(0U, buf, "666F6F");
  assert(r.status == EXESS_NO_SPACE);
  assert(r.read_count == 2U);
  assert(r.write_count == 0U);
  assert(buf[0] == 1);

  r = exess_read_hex(1U, buf, "666F6F");
  assert(r.status == EXESS_NO_SPACE);
  assert(r.read_count == 4U);
  assert(r.write_count == 1U);
  assert(buf[0] == 'f');

  r = exess_read_hex(2U, buf, "666F6F");
  assert(r.status == EXESS_NO_SPACE);
  assert(r.read_count == 6U);
  assert(r.write_count == 2U);
  assert(buf[0] == 'f');
  assert(buf[1] == 'o');

  r = exess_read_hex(3U, buf, "666F6F");
  assert(r.status == EXESS_SUCCESS);
  assert(r.read_count == 6U);
  assert(r.write_count == 3U);
  assert(!strncmp(buf, "foo", 3));
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
    uint8_t* const decoded = (uint8_t*)malloc(size);

    // Decode and check that data matches the original input
    const ExessVariableResult r = exess_read_hex(size, decoded, str);
    assert(!r.status);
    assert(r.read_count == str_len);
    assert(r.write_count == size);
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
