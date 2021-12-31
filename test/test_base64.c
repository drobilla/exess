// Copyright 2011-2021 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

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
  char buf[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};

  ExessVariableResult r = exess_read_base64(sizeof(buf), buf, string);
  assert(r.status == expected_status);
  assert(r.read_count == expected_count);
  assert(r.status || r.write_count == expected_value_size);
  if (expected_value_length > 0) {
    assert(!strncmp(buf, expected_value, expected_value_length));
    assert(r.write_count <= exess_base64_decoded_size(strlen(string)));
  }
}

static void
test_rfc4648_cases(void)
{
  char buf[9] = {1, 2, 3, 4, 5, 6, 7, 8, 9};

  assert(!exess_write_base64(6, "foobar", sizeof(buf), buf).status);
  assert(!strcmp(buf, "Zm9vYmFy"));

  assert(!exess_write_base64(5, "fooba", sizeof(buf), buf).status);
  assert(!strcmp(buf, "Zm9vYmE="));

  assert(!exess_write_base64(4, "foob", sizeof(buf), buf).status);
  assert(!strcmp(buf, "Zm9vYg=="));

  assert(!exess_write_base64(3, "foo", sizeof(buf), buf).status);
  assert(!strcmp(buf, "Zm9v"));

  assert(!exess_write_base64(2, "fo", sizeof(buf), buf).status);
  assert(!strcmp(buf, "Zm8="));

  assert(!exess_write_base64(1, "f", sizeof(buf), buf).status);
  assert(!strcmp(buf, "Zg=="));
}

static void
test_whitespace(void)
{
  check_read("Zm9vYmFy", EXESS_SUCCESS, 6, "foobar", 6, 8);
  check_read(" Zm9vYmFy", EXESS_SUCCESS, 6, "foobar", 6, 9);
  check_read("Z\fm9vYmFy", EXESS_SUCCESS, 6, "foobar", 6, 9);
  check_read("Zm\n9vYmFy", EXESS_SUCCESS, 6, "foobar", 6, 9);
  check_read("Zm9\rvYmFy", EXESS_SUCCESS, 6, "foobar", 6, 9);
  check_read("Zm9v\tYmFy", EXESS_SUCCESS, 6, "foobar", 6, 9);
  check_read("Zm9vY\vmFy", EXESS_SUCCESS, 6, "foobar", 6, 9);
  check_read(" \f\n\r\t\vZm9vYmFy", EXESS_SUCCESS, 6, "foobar", 6, 14);
  check_read("Zm9vYmFy \f\n\r\t\v", EXESS_SUCCESS, 6, "foobar", 6, 14);
}

static void
test_syntax_errors(void)
{
  check_read("Z", EXESS_EXPECTED_BASE64, 0, NULL, 0, 1);
  check_read("ZZ", EXESS_EXPECTED_BASE64, 0, NULL, 0, 2);
  check_read("ZZZ", EXESS_EXPECTED_BASE64, 0, NULL, 0, 3);

  check_read("=ZZZ", EXESS_BAD_VALUE, 0, NULL, 0, 4);
  check_read("Z=ZZ", EXESS_BAD_VALUE, 0, NULL, 0, 4);
  check_read("ZZ=Z", EXESS_BAD_VALUE, 0, NULL, 0, 4);

  check_read("!m9vYmFy", EXESS_EXPECTED_BASE64, 0, NULL, 0, 0);
  check_read("Z!9vYmFy", EXESS_EXPECTED_BASE64, 0, NULL, 0, 1);
  check_read("Zm!vYmFy", EXESS_EXPECTED_BASE64, 0, NULL, 0, 2);
  check_read("Zm9!YmFy", EXESS_EXPECTED_BASE64, 0, NULL, 0, 3);
  check_read("Zm9v!mFy", EXESS_EXPECTED_BASE64, 0, NULL, 3, 4);
  check_read("Zm9vY!Fy", EXESS_EXPECTED_BASE64, 0, NULL, 3, 5);
  check_read("Zm9vYm!y", EXESS_EXPECTED_BASE64, 0, NULL, 3, 6);
  check_read("Zm9vYmF!", EXESS_EXPECTED_BASE64, 0, NULL, 3, 7);
}

static void
test_read_overflow(void)
{
  char buf[3] = {0, 0, 0};

  ExessVariableResult r = exess_read_base64(0u, buf, "Zm9v");
  assert(r.status == EXESS_NO_SPACE);
  assert(r.read_count == 4);
  assert(r.write_count == 0);

  r = exess_read_base64(1u, buf, "Zm9v");
  assert(r.status == EXESS_NO_SPACE);
  assert(r.read_count == 4u);
  assert(r.write_count == 0u);
  assert(!buf[0]);

  r = exess_read_base64(2u, buf, "Zm9v");
  assert(r.status == EXESS_NO_SPACE);
  assert(r.read_count == 4u);
  assert(r.write_count == 0u);
  assert(!buf[0]);

  r = exess_read_base64(3u, buf, "Zm9v");
  assert(r.status == EXESS_SUCCESS);
  assert(r.read_count == 4u);
  assert(r.write_count == 3u);
  assert(!strncmp(buf, "foo", 3));
}

static void
test_write_overflow(void)
{
  char buf[5] = {1, 2, 3, 4, 5};

  assert(exess_write_base64(3, "foo", 0, buf).status == EXESS_NO_SPACE);
  assert(exess_write_base64(3, "foo", 1, buf).status == EXESS_NO_SPACE);
  assert(exess_write_base64(3, "foo", 2, buf).status == EXESS_NO_SPACE);
  assert(exess_write_base64(3, "foo", 3, buf).status == EXESS_NO_SPACE);
  assert(exess_write_base64(3, "foo", 4, buf).status == EXESS_NO_SPACE);
  assert(exess_write_base64(3, "foo", 5, buf).status == EXESS_SUCCESS);
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
    const size_t str_len = exess_write_base64(size, data, 0, NULL).count;
    char* const  str     = (char*)malloc(str_len + 1);

    // Encode data to string buffer
    assert(!exess_write_base64(size, data, str_len + 1, str).status);
    assert(strlen(str) == str_len);
    assert(str_len % 4 == 0);

    // Allocate buffer for decoded data with the same size as the input
    uint8_t* const decoded = (uint8_t*)malloc(size);

    // Decode and check that data matches the original input
    const ExessVariableResult r = exess_read_base64(size, decoded, str);
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
  test_rfc4648_cases();
  test_whitespace();
  test_syntax_errors();
  test_read_overflow();
  test_write_overflow();
  test_round_trip();

  return 0;
}
