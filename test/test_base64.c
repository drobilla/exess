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
check(const size_t      expected_read_count,
      const char* const string,
      const ExessStatus expected_status,
      const size_t      expected_write_count,
      const char* const expected_value)
{
  char buf[9] = {1, 2, 3, 4, 5, 6, 7, 8, 9};

  ExessVariableResult r = exess_read_base64(sizeof(buf), buf, string);
  assert(r.status == expected_status);
  assert(r.read_count == expected_read_count);
  assert(r.write_count == expected_write_count);
  assert(r.write_count > 0 || buf[0] == 1);
  if (expected_write_count > 0) {
    assert(!strncmp(buf, expected_value, expected_write_count));
    assert(r.write_count <= exess_decoded_base64_size(strlen(string)));
  }
}

static void
test_rfc4648_cases(void)
{
  char buf[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

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
  check(8, "Zm9vYmFy", EXESS_SUCCESS, 6, "foobar");
  check(9, "\tZm9vYmFy", EXESS_SUCCESS, 6, "foobar");
  check(9, "Z\nm9vYmFy", EXESS_SUCCESS, 6, "foobar");
  check(9, "Zm\r9vYmFy", EXESS_SUCCESS, 6, "foobar");
  check(9, "Zm9 vYmFy", EXESS_SUCCESS, 6, "foobar");
  check(9, "Zm9v\tYmFy", EXESS_SUCCESS, 6, "foobar");
  check(9, "Zm9vY\nmFy", EXESS_SUCCESS, 6, "foobar");
  check(12, "\t\n\r Zm9vYmFy", EXESS_SUCCESS, 6, "foobar");
  check(12, "Zm9vYmFy\t\n\r ", EXESS_SUCCESS, 6, "foobar");
}

static void
test_syntax_errors(void)
{
  check(1, "Z", EXESS_EXPECTED_BASE64, 0, NULL);
  check(2, "ZZ", EXESS_EXPECTED_BASE64, 0, NULL);
  check(3, "ZZZ", EXESS_EXPECTED_BASE64, 0, NULL);

  check(4, "=ZZZ", EXESS_BAD_VALUE, 0, NULL);
  check(4, "Z=ZZ", EXESS_BAD_VALUE, 0, NULL);
  check(4, "ZZ=Z", EXESS_BAD_VALUE, 0, NULL);

  check(0, "!m9vYmFy", EXESS_EXPECTED_BASE64, 0, NULL);
  check(1, "Z!9vYmFy", EXESS_EXPECTED_BASE64, 0, NULL);
  check(2, "Zm!vYmFy", EXESS_EXPECTED_BASE64, 0, NULL);
  check(3, "Zm9!YmFy", EXESS_EXPECTED_BASE64, 0, NULL);
  check(4, "Zm9v!mFy", EXESS_EXPECTED_BASE64, 3, "foo");
  check(5, "Zm9vY!Fy", EXESS_EXPECTED_BASE64, 3, "foo");
  check(6, "Zm9vYm!y", EXESS_EXPECTED_BASE64, 3, "foo");
  check(7, "Zm9vYmF!", EXESS_EXPECTED_BASE64, 3, "foo");
}

static void
test_read_overflow(void)
{
  char buf[3] = {1, 2, 3};

  ExessVariableResult r = exess_read_base64(0U, buf, "Zm9v");
  assert(r.status == EXESS_NO_SPACE);
  assert(r.read_count == 4);
  assert(r.write_count == 0);
  assert(buf[0] == 1);

  r = exess_read_base64(1U, buf, "Zm9v");
  assert(r.status == EXESS_NO_SPACE);
  assert(r.read_count == 4U);
  assert(r.write_count == 0U);
  assert(buf[0] == 1);

  r = exess_read_base64(2U, buf, "Zm9v");
  assert(r.status == EXESS_NO_SPACE);
  assert(r.read_count == 4U);
  assert(r.write_count == 0U);
  assert(buf[0] == 1);

  r = exess_read_base64(3U, buf, "Zm9v");
  assert(r.status == EXESS_SUCCESS);
  assert(r.read_count == 4U);
  assert(r.write_count == 3U);
  assert(!strncmp(buf, "foo", 3));
}

static void
test_write_overflow(void)
{
  char buf[5] = {1, 2, 3, 4, 5};

  assert(exess_write_base64(3, "foo", 0, buf).status == EXESS_NO_SPACE);
  assert(buf[0] == 1);
  assert(exess_write_base64(3, "foo", 1, buf).status == EXESS_NO_SPACE);
  assert(buf[0] == 1);
  assert(exess_write_base64(3, "foo", 2, buf).status == EXESS_NO_SPACE);
  assert(buf[0] == 1);
  assert(exess_write_base64(3, "foo", 3, buf).status == EXESS_NO_SPACE);
  assert(buf[0] == 1);
  assert(exess_write_base64(3, "foo", 4, buf).status == EXESS_NO_SPACE);
  assert(buf[0] == 1);
  assert(exess_write_base64(3, "foo", 5, buf).status == EXESS_SUCCESS);
  assert(!strncmp(buf, "Zm9v", 4));
}

static void
test_round_trip(void)
{
  for (size_t size = 1; size < 256; ++size) {
    // Allocate and generate data
    uint8_t* const data = (uint8_t*)malloc(size);
    assert(data);
    for (size_t i = 0; i < size; ++i) {
      data[i] = (uint8_t)((size + i) % 256);
    }

    // Allocate buffer for encoding with minimum required size
    const size_t str_len = exess_write_base64(size, data, 0, NULL).count;
    char* const  str     = (char*)malloc(str_len + 1);
    assert(str);

    // Encode data to string buffer
    assert(!exess_write_base64(size, data, str_len + 1, str).status);
    assert(strlen(str) == str_len);
    assert(str_len % 4 == 0);

    // Allocate buffer for decoded data with the same size as the input
    uint8_t* const decoded = (uint8_t*)malloc(size);
    assert(decoded);

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
