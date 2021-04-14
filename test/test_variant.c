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

#include "float_test_data.h"

#include "exess/exess.h"

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#define CHECK_POINTEE_EQUALS(p, v) assert((p) && (*(p) == (v)))

static const ExessDuration duration = {14,
                                       3 * 24 * 60 * 60 + 4 * 60 * 60 + 5 * 60 +
                                         6,
                                       0};

static const ExessDateTime datetime = {2001, 2, 3, false, 4, 5, 6, 0};
static const ExessTime     time     = {{0}, 1, 2, 3, 0};
static const ExessDate     date     = {2001, 2, 3, {0}};

static void
check_read(ExessVariant* const variant,
           const ExessDatatype datatype,
           const char* const   string,
           const ExessStatus   expected_status,
           const size_t        expected_count)
{
  const ExessResult r = exess_read_variant(variant, datatype, string);

  assert(r.status == expected_status);
  assert(r.count == expected_count);
  assert(variant->datatype == datatype);
}

static void
test_read_variant(void)
{
  ExessVariant variant;

  check_read(&variant, EXESS_NOTHING, "42", EXESS_UNSUPPORTED, 0);

  check_read(&variant, EXESS_DECIMAL, "1.2", EXESS_SUCCESS, 3);
  assert(double_matches(*exess_get_double(&variant), 1.2));

  check_read(&variant, EXESS_DOUBLE, "3.4", EXESS_SUCCESS, 3);
  assert(double_matches(variant.value.as_double, 3.4));

  check_read(&variant, EXESS_FLOAT, "5.6", EXESS_SUCCESS, 3);
  assert(float_matches(variant.value.as_float, 5.6f));

  check_read(&variant, EXESS_BOOLEAN, "true", EXESS_SUCCESS, 4);
  assert(variant.value.as_bool);

  check_read(&variant, EXESS_INTEGER, "7", EXESS_SUCCESS, 1);
  assert(variant.value.as_long == 7);

  check_read(
    &variant, EXESS_NON_POSITIVE_INTEGER, "f", EXESS_EXPECTED_DIGIT, 0);
  check_read(&variant, EXESS_NON_POSITIVE_INTEGER, "1", EXESS_OUT_OF_RANGE, 1);
  check_read(&variant, EXESS_NON_POSITIVE_INTEGER, "-8", EXESS_SUCCESS, 2);
  assert(variant.value.as_long == -8);

  check_read(&variant, EXESS_NEGATIVE_INTEGER, "f", EXESS_EXPECTED_DIGIT, 0);
  check_read(&variant, EXESS_NEGATIVE_INTEGER, "1", EXESS_OUT_OF_RANGE, 1);
  check_read(&variant, EXESS_NEGATIVE_INTEGER, "-9", EXESS_SUCCESS, 2);
  assert(variant.value.as_long == -9);

  check_read(&variant, EXESS_LONG, "10", EXESS_SUCCESS, 2);
  assert(variant.value.as_long == 10);

  check_read(&variant, EXESS_INT, "11", EXESS_SUCCESS, 2);
  assert(variant.value.as_int == 11);

  check_read(&variant, EXESS_SHORT, "12", EXESS_SUCCESS, 2);
  assert(variant.value.as_short == 12);

  check_read(&variant, EXESS_BYTE, "13", EXESS_SUCCESS, 2);
  assert(variant.value.as_byte == 13);

  check_read(&variant, EXESS_ULONG, "14", EXESS_SUCCESS, 2);
  assert(variant.value.as_long == 14);

  check_read(&variant, EXESS_UINT, "15", EXESS_SUCCESS, 2);
  assert(variant.value.as_int == 15);

  check_read(&variant, EXESS_USHORT, "16", EXESS_SUCCESS, 2);
  assert(variant.value.as_short == 16);

  check_read(&variant, EXESS_UBYTE, "17", EXESS_SUCCESS, 2);
  assert(variant.value.as_byte == 17);

  check_read(&variant, EXESS_POSITIVE_INTEGER, "-1", EXESS_EXPECTED_DIGIT, 0);
  check_read(&variant, EXESS_POSITIVE_INTEGER, "0", EXESS_OUT_OF_RANGE, 1);
  check_read(&variant, EXESS_POSITIVE_INTEGER, "18", EXESS_SUCCESS, 2);
  assert(variant.value.as_long == 18);

  check_read(&variant, EXESS_DATE, "2001-01-02", EXESS_SUCCESS, 10);
  assert(variant.value.as_date.year == 2001);
  assert(variant.value.as_date.month == 1);
  assert(variant.value.as_date.day == 2);

  check_read(&variant, EXESS_TIME, "12:15:01.25", EXESS_SUCCESS, 11);
  assert(variant.value.as_time.hour == 12);
  assert(variant.value.as_time.minute == 15);
  assert(variant.value.as_time.second == 1);
  assert(variant.value.as_time.nanosecond == 250000000);

  char blob_data[] = {0, 0, 0};

  variant.datatype           = EXESS_HEX;
  variant.value.as_blob.size = sizeof(blob_data);
  variant.value.as_blob.data = blob_data;
  check_read(&variant, EXESS_HEX, "666F6F", EXESS_SUCCESS, 6);
  assert(!strncmp(blob_data, "foo", sizeof(blob_data)));

  variant.datatype           = EXESS_BASE64;
  variant.value.as_blob.size = sizeof(blob_data);
  variant.value.as_blob.data = blob_data;
  check_read(&variant, EXESS_BASE64, "Zm9v", EXESS_SUCCESS, 4);
  assert(!strncmp(blob_data, "foo", sizeof(blob_data)));
}

static void
test_variant_string_length(void)
{
  const ExessVariant variant = {EXESS_DECIMAL, {.as_double = 12.3456}};

  assert(exess_write_variant(variant, 0, NULL).count == 7);
}

static void
check_write(const ExessVariant value,
            const ExessStatus  expected_status,
            const size_t       buf_size,
            const char* const  expected_string)
{
  char buf[328] = {42};

  assert(buf_size <= sizeof(buf));

  const ExessResult r = exess_write_variant(value, buf_size, buf);
  assert(r.status == expected_status);
  if (buf_size > 0) {
    assert(r.count == strlen(buf));
    assert(!strcmp(buf, expected_string));
  }
}

static void
test_write_variant(void)
{
  char            blob_data[] = {'f', 'o', 'o'};
  const ExessBlob blob        = {sizeof(blob_data), blob_data};

  const ExessVariant a_nothing  = exess_make_nothing(EXESS_SUCCESS);
  const ExessVariant a_bool     = exess_make_boolean(true);
  const ExessVariant a_decimal  = exess_make_decimal(1.2);
  const ExessVariant a_double   = exess_make_double(3.4);
  const ExessVariant a_float    = exess_make_float(5.6f);
  const ExessVariant a_long     = exess_make_long(7);
  const ExessVariant a_int      = exess_make_int(8);
  const ExessVariant a_short    = exess_make_short(9);
  const ExessVariant a_byte     = exess_make_byte(10);
  const ExessVariant a_ulong    = exess_make_ulong(11);
  const ExessVariant a_uint     = exess_make_uint(12);
  const ExessVariant a_ushort   = exess_make_ushort(13);
  const ExessVariant a_ubyte    = exess_make_ubyte(14);
  const ExessVariant a_duration = exess_make_duration(duration);
  const ExessVariant a_datetime = exess_make_datetime(datetime);
  const ExessVariant a_time     = exess_make_time(time);
  const ExessVariant a_date     = exess_make_date(date);
  const ExessVariant a_hex      = exess_make_hex(blob);
  const ExessVariant a_base64   = exess_make_base64(blob);

  check_write(a_nothing, EXESS_BAD_VALUE, 0, "");
  check_write(a_nothing, EXESS_BAD_VALUE, 1, "");
  check_write(a_decimal, EXESS_SUCCESS, 4, "1.2");
  check_write(a_double, EXESS_SUCCESS, 6, "3.4E0");
  check_write(a_float, EXESS_SUCCESS, 12, "5.5999999E0");
  check_write(a_bool, EXESS_SUCCESS, 5, "true");
  check_write(a_long, EXESS_SUCCESS, 2, "7");
  check_write(a_int, EXESS_SUCCESS, 2, "8");
  check_write(a_short, EXESS_SUCCESS, 2, "9");
  check_write(a_byte, EXESS_SUCCESS, 3, "10");
  check_write(a_ulong, EXESS_SUCCESS, 3, "11");
  check_write(a_uint, EXESS_SUCCESS, 3, "12");
  check_write(a_ushort, EXESS_SUCCESS, 3, "13");
  check_write(a_ubyte, EXESS_SUCCESS, 3, "14");
  check_write(a_duration, EXESS_SUCCESS, 15, "P1Y2M3DT4H5M6S");
  check_write(a_datetime, EXESS_SUCCESS, 40, "2001-02-03T04:05:06");
  check_write(a_time, EXESS_SUCCESS, 40, "01:02:03Z");
  check_write(a_date, EXESS_SUCCESS, 40, "2001-02-03Z");
  check_write(a_hex, EXESS_SUCCESS, 7, "666F6F");
  check_write(a_base64, EXESS_SUCCESS, 5, "Zm9v");

  const ExessBlob null_blob = {0, NULL};

  const ExessVariant null_hex = exess_make_hex(null_blob);
  check_write(null_hex, EXESS_BAD_VALUE, 99, "");

  const ExessVariant null_base64 = exess_make_base64(null_blob);
  check_write(null_base64, EXESS_BAD_VALUE, 99, "");
}

static void
test_make_get(void)
{
  char            blob_data[] = {'f', 'o', 'o'};
  const ExessBlob blob        = {sizeof(blob_data), blob_data};

  const ExessVariant a_nothing  = exess_make_nothing(EXESS_NO_SPACE);
  const ExessVariant a_bool     = exess_make_boolean(true);
  const ExessVariant a_decimal  = exess_make_decimal(1.2);
  const ExessVariant a_double   = exess_make_double(3.4);
  const ExessVariant a_float    = exess_make_float(5.6f);
  const ExessVariant a_long     = exess_make_long(7);
  const ExessVariant a_int      = exess_make_int(8);
  const ExessVariant a_short    = exess_make_short(9);
  const ExessVariant a_byte     = exess_make_byte(10);
  const ExessVariant a_ulong    = exess_make_ulong(11);
  const ExessVariant a_uint     = exess_make_uint(12);
  const ExessVariant a_ushort   = exess_make_ushort(13);
  const ExessVariant a_ubyte    = exess_make_ubyte(14);
  const ExessVariant a_duration = exess_make_duration(duration);
  const ExessVariant a_datetime = exess_make_datetime(datetime);
  const ExessVariant a_time     = exess_make_time(time);
  const ExessVariant a_date     = exess_make_date(date);
  const ExessVariant a_hex      = exess_make_hex(blob);
  const ExessVariant a_base64   = exess_make_base64(blob);

  // Different types as status
  assert(exess_get_status(&a_nothing) == EXESS_NO_SPACE);
  assert(exess_get_status(&a_bool) == EXESS_SUCCESS);

  // Basic successful get
  CHECK_POINTEE_EQUALS(exess_get_boolean(&a_bool), true);
  assert(double_matches(*exess_get_double(&a_decimal), 1.2));
  assert(double_matches(*exess_get_double(&a_double), 3.4));
  assert(float_matches(*exess_get_float(&a_float), 5.6f));
  CHECK_POINTEE_EQUALS(exess_get_long(&a_long), 7);
  CHECK_POINTEE_EQUALS(exess_get_int(&a_int), 8);
  CHECK_POINTEE_EQUALS(exess_get_short(&a_short), 9);
  CHECK_POINTEE_EQUALS(exess_get_byte(&a_byte), 10);
  CHECK_POINTEE_EQUALS(exess_get_ulong(&a_ulong), 11u);
  CHECK_POINTEE_EQUALS(exess_get_uint(&a_uint), 12u);
  CHECK_POINTEE_EQUALS(exess_get_ushort(&a_ushort), 13u);
  CHECK_POINTEE_EQUALS(exess_get_ubyte(&a_ubyte), 14u);
  assert(!memcmp(exess_get_duration(&a_duration), &duration, sizeof(duration)));
  assert(!memcmp(exess_get_datetime(&a_datetime), &datetime, sizeof(datetime)));
  assert(!memcmp(exess_get_time(&a_time), &time, sizeof(time)));
  assert(!memcmp(exess_get_date(&a_date), &date, sizeof(date)));
  assert(exess_get_blob(&a_hex)->size == sizeof(blob_data));
  assert(exess_get_blob(&a_hex)->data == blob_data);
  assert(exess_get_blob(&a_base64)->size == sizeof(blob_data));
  assert(exess_get_blob(&a_base64)->data == blob_data);

  // Unsuccessful get
  assert(!exess_get_boolean(&a_int));
  assert(!exess_get_double(&a_int));
  assert(!exess_get_float(&a_int));
  assert(!exess_get_long(&a_bool));
  assert(!exess_get_int(&a_bool));
  assert(!exess_get_short(&a_int));
  assert(!exess_get_byte(&a_int));
  assert(!exess_get_ulong(&a_int));
  assert(!exess_get_uint(&a_int));
  assert(!exess_get_ushort(&a_int));
  assert(!exess_get_ubyte(&a_int));
  assert(!exess_get_duration(&a_int));
  assert(!exess_get_datetime(&a_int));
  assert(!exess_get_time(&a_int));
  assert(!exess_get_date(&a_int));
  assert(!exess_get_blob(&a_int));
}

static inline bool
less(const int cmp)
{
  return cmp < 0;
}

static inline bool
equal(const int cmp)
{
  return cmp == 0;
}

static inline bool
greater(const int cmp)
{
  return cmp > 0;
}

static void
check_comparison(const ExessDatatype lhs_datatype,
                 const char* const   lhs_string,
                 const ExessDatatype rhs_datatype,
                 const char* const   rhs_string,
                 bool (*pred)(int))
{
  char            lhs_buf[1024] = {0};
  char            rhs_buf[1024] = {0};
  const ExessBlob lhs_blob      = {sizeof(lhs_buf), lhs_buf};
  const ExessBlob rhs_blob      = {sizeof(rhs_buf), rhs_buf};
  ExessVariant    lhs           = exess_make_hex(lhs_blob);
  ExessVariant    rhs           = exess_make_hex(rhs_blob);

  const ExessResult l = exess_read_variant(&lhs, lhs_datatype, lhs_string);
  const ExessResult r = exess_read_variant(&rhs, rhs_datatype, rhs_string);

  assert(lhs_datatype == EXESS_NOTHING || !l.status);
  assert(lhs.datatype == lhs_datatype);
  assert(rhs_datatype == EXESS_NOTHING || !r.status);
  assert(rhs.datatype == rhs_datatype);
  assert(pred(exess_compare(lhs, rhs)));
}

static void
check_comparisons(const ExessDatatype datatype,
                  const char* const   low,
                  const char* const   mid,
                  const char* const   high)
{
  check_comparison(datatype, low, datatype, mid, less);
  check_comparison(datatype, mid, datatype, mid, equal);
  check_comparison(datatype, high, datatype, mid, greater);
}

static void
test_compare(void)
{
  // Cross-type and nothing
  check_comparison(EXESS_NOTHING, "", EXESS_NOTHING, "", equal);
  check_comparison(EXESS_NOTHING, "", EXESS_INT, "0", less);
  check_comparison(EXESS_INT, "0", EXESS_NOTHING, "", greater);
  check_comparison(EXESS_BOOLEAN, "true", EXESS_INT, "0", less);
  check_comparison(EXESS_TIME, "12:00:00", EXESS_INT, "42", greater);

  // All boolean cases
  check_comparison(EXESS_BOOLEAN, "false", EXESS_BOOLEAN, "true", less);
  check_comparison(EXESS_BOOLEAN, "false", EXESS_BOOLEAN, "1", less);
  check_comparison(EXESS_BOOLEAN, "0", EXESS_BOOLEAN, "true", less);
  check_comparison(EXESS_BOOLEAN, "0", EXESS_BOOLEAN, "1", less);
  check_comparison(EXESS_BOOLEAN, "false", EXESS_BOOLEAN, "false", equal);
  check_comparison(EXESS_BOOLEAN, "false", EXESS_BOOLEAN, "0", equal);
  check_comparison(EXESS_BOOLEAN, "0", EXESS_BOOLEAN, "0", equal);
  check_comparison(EXESS_BOOLEAN, "0", EXESS_BOOLEAN, "false", equal);
  check_comparison(EXESS_BOOLEAN, "true", EXESS_BOOLEAN, "true", equal);
  check_comparison(EXESS_BOOLEAN, "true", EXESS_BOOLEAN, "1", equal);
  check_comparison(EXESS_BOOLEAN, "1", EXESS_BOOLEAN, "1", equal);
  check_comparison(EXESS_BOOLEAN, "1", EXESS_BOOLEAN, "true", equal);
  check_comparison(EXESS_BOOLEAN, "true", EXESS_BOOLEAN, "false", greater);
  check_comparison(EXESS_BOOLEAN, "true", EXESS_BOOLEAN, "0", greater);
  check_comparison(EXESS_BOOLEAN, "1", EXESS_BOOLEAN, "0", greater);

  // Numbers
  check_comparisons(EXESS_DECIMAL, "-9.0", "0.0", "9.0");
  check_comparisons(EXESS_DOUBLE, "1.0E-1", "1.0E0", "1.0E1");
  check_comparisons(EXESS_FLOAT, "-1.0E-1", "1.0E0", "1.0E1");
  check_comparisons(EXESS_INTEGER, "-9", "0", "9");
  check_comparisons(EXESS_NON_POSITIVE_INTEGER, "-9", "-5", "0");
  check_comparisons(EXESS_NEGATIVE_INTEGER, "-9", "-5", "-1");
  check_comparisons(EXESS_LONG, "-9", "0", "9");
  check_comparisons(EXESS_INT, "-9", "0", "9");
  check_comparisons(EXESS_SHORT, "-9", "0", "9");
  check_comparisons(EXESS_BYTE, "-9", "0", "9");
  check_comparisons(EXESS_NON_NEGATIVE_INTEGER, "0", "5", "9");
  check_comparisons(EXESS_ULONG, "0", "5", "9");
  check_comparisons(EXESS_UINT, "0", "5", "9");
  check_comparisons(EXESS_USHORT, "0", "5", "9");
  check_comparisons(EXESS_UBYTE, "0", "5", "9");
  check_comparisons(EXESS_POSITIVE_INTEGER, "1", "5", "9");

  // Duration
  check_comparisons(EXESS_DURATION, "-P9Y", "P0Y", "P9Y");
  check_comparisons(EXESS_DURATION, "-P9M", "P0M", "P9M");
  check_comparisons(EXESS_DURATION, "-P9D", "P0D", "P9D");
  check_comparisons(EXESS_DURATION, "-PT9H", "PT0H", "PT9H");
  check_comparisons(EXESS_DURATION, "-PT9M", "PT0M", "PT9M");
  check_comparisons(EXESS_DURATION, "-PT9S", "PT0S", "PT9S");
  check_comparisons(EXESS_DURATION, "-PT0.9S", "PT0.0S", "PT0.9S");

  // Datetime

  // Equality
  check_comparison(EXESS_DATETIME,
                   "2001-02-03T12:13:14",
                   EXESS_DATETIME,
                   "2001-02-03T12:13:14",
                   equal);
  check_comparison(EXESS_DATETIME,
                   "2001-02-03T12:13:14Z",
                   EXESS_DATETIME,
                   "2001-02-03T12:13:14Z",
                   equal);

  // All local
  check_comparisons(EXESS_DATETIME,
                    "2001-02-03T12:13:14.15",
                    "2001-02-03T12:13:14.16",
                    "2001-02-03T12:13:14.17");
  check_comparisons(EXESS_DATETIME,
                    "2001-02-03T12:13:14",
                    "2001-02-03T12:13:15",
                    "2001-02-03T12:13:16");
  check_comparisons(EXESS_DATETIME,
                    "2001-02-03T12:13:14",
                    "2001-02-03T12:14:14",
                    "2001-02-03T12:15:14");
  check_comparisons(EXESS_DATETIME,
                    "2001-02-03T12:13:14",
                    "2001-02-03T13:13:14",
                    "2001-02-03T14:13:14");
  check_comparisons(EXESS_DATETIME,
                    "2001-02-03T12:13:14",
                    "2001-02-04T12:13:14",
                    "2001-02-05T12:13:14");
  check_comparisons(EXESS_DATETIME,
                    "2001-02-03T12:13:14",
                    "2001-03-03T12:13:14",
                    "2001-04-03T12:13:14");
  check_comparisons(EXESS_DATETIME,
                    "2001-02-03T12:13:14",
                    "2002-02-03T12:13:14",
                    "2003-02-03T12:13:14");

  // All UTC
  check_comparisons(EXESS_DATETIME,
                    "2001-02-03T12:13:14.15Z",
                    "2001-02-03T12:13:14.16Z",
                    "2001-02-03T12:13:14.17Z");
  check_comparisons(EXESS_DATETIME,
                    "2001-02-03T12:13:14Z",
                    "2001-02-03T12:13:15Z",
                    "2001-02-03T12:13:16Z");
  check_comparisons(EXESS_DATETIME,
                    "2001-02-03T12:13:14Z",
                    "2001-02-03T12:14:14Z",
                    "2001-02-03T12:15:14Z");
  check_comparisons(EXESS_DATETIME,
                    "2001-02-03T12:13:14Z",
                    "2001-02-03T13:13:14Z",
                    "2001-02-03T14:13:14Z");
  check_comparisons(EXESS_DATETIME,
                    "2001-02-03T12:13:14Z",
                    "2001-02-04T12:13:14Z",
                    "2001-02-05T12:13:14Z");
  check_comparisons(EXESS_DATETIME,
                    "2001-02-03T12:13:14Z",
                    "2001-03-03T12:13:14Z",
                    "2001-04-03T12:13:14Z");
  check_comparisons(EXESS_DATETIME,
                    "2001-02-03T12:13:14Z",
                    "2002-02-03T12:13:14Z",
                    "2003-02-03T12:13:14Z");

  // Local and UTC determinate
  check_comparisons(EXESS_DATETIME,
                    "2001-02-03T12:13:14Z",
                    "2001-02-04T02:13:15",
                    "2001-02-04T16:13:15Z");
  check_comparisons(EXESS_DATETIME,
                    "2001-02-03T12:13:14Z",
                    "2001-02-04T12:13:14",
                    "2001-02-05T12:13:14Z");
  check_comparisons(EXESS_DATETIME,
                    "2001-02-03T12:13:14",
                    "2001-02-04T02:13:15Z",
                    "2001-02-04T16:13:16");
  check_comparison(EXESS_DATETIME,
                   "2001-02-03T12:13:14Z",
                   EXESS_DATETIME,
                   "2001-02-03T12:13:14",
                   greater);

  // Local and UTC determinate (example from spec)
  check_comparison(EXESS_DATETIME,
                   "2000-01-15T12:00:00",
                   EXESS_DATETIME,
                   "2000-01-16T12:00:00Z",
                   less);

  // Local and UTC indeterminate (examples from spec, local is first here)
  check_comparison(EXESS_DATETIME,
                   "2000-01-01T12:00:00",
                   EXESS_DATETIME,
                   "1999-12-31T23:00:00Z",
                   less);
  check_comparison(EXESS_DATETIME,
                   "2000-01-16T12:00:00",
                   EXESS_DATETIME,
                   "2000-01-16T12:00:00Z",
                   less);
  check_comparison(EXESS_DATETIME,
                   "2000-01-16T00:00:00",
                   EXESS_DATETIME,
                   "2000-01-16T12:00:00Z",
                   less);

  // Time

  // Equality
  check_comparison(EXESS_TIME, "12:13:14", EXESS_TIME, "12:13:14", equal);
  check_comparison(EXESS_TIME, "12:13:14Z", EXESS_TIME, "12:13:14Z", equal);
  check_comparison(EXESS_TIME, "12:13:14", EXESS_TIME, "12:13:14", equal);
  check_comparison(
    EXESS_TIME, "12:13:14+00:00", EXESS_TIME, "12:13:14+00:00", equal);
  check_comparison(EXESS_TIME, "12:13:14", EXESS_TIME, "12:13:14", equal);
  check_comparison(
    EXESS_TIME, "12:13:14-00:00", EXESS_TIME, "12:13:14-00:00", equal);

  // All local
  check_comparisons(EXESS_TIME, "12:13:14.15", "12:13:14.16", "12:13:14.17");
  check_comparisons(EXESS_TIME, "12:13:14", "12:13:15", "12:13:16");
  check_comparisons(EXESS_TIME, "12:13:14", "12:14:14", "12:15:14");
  check_comparisons(EXESS_TIME, "12:13:14", "13:13:14", "14:13:14");

  // All UTC
  check_comparisons(EXESS_TIME, "12:13:14.15Z", "12:13:14.16Z", "12:13:14.17Z");
  check_comparisons(EXESS_TIME, "12:13:14Z", "12:13:15Z", "12:13:16Z");
  check_comparisons(EXESS_TIME, "12:13:14Z", "12:14:14Z", "12:15:14Z");
  check_comparisons(EXESS_TIME, "12:13:14Z", "13:13:14Z", "14:13:14Z");

  // Time zones
  check_comparisons(
    EXESS_TIME, "12:13:14+03:00", "12:13:14+02:00", "12:13:14+01:00");
  check_comparisons(
    EXESS_TIME, "12:13:14+03:00", "12:13:13+01:00", "12:13:13+00:00");

  // Local and UTC determinate
  check_comparisons(EXESS_TIME, "12:13:14", "02:13:15Z", "16:13:16");
  check_comparison(EXESS_TIME, "12:13:14Z", EXESS_TIME, "12:13:14", greater);

  // Local and UTC determinate
  check_comparison(EXESS_TIME, "01:00:00Z", EXESS_TIME, "15:00:01", less);
  check_comparison(EXESS_TIME, "15:00:00Z", EXESS_TIME, "01:00:00", greater);
  check_comparison(EXESS_TIME, "15:00:01", EXESS_TIME, "01:00:00Z", greater);
  check_comparison(EXESS_TIME, "01:00:00", EXESS_TIME, "15:00:00Z", less);

  // Local and UTC indeterminate (local is first here)
  check_comparison(EXESS_TIME, "12:00:00", EXESS_TIME, "12:00:00Z", less);
  check_comparison(EXESS_TIME, "00:00:00", EXESS_TIME, "12:00:00Z", less);
  check_comparison(EXESS_TIME, "12:00:00Z", EXESS_TIME, "12:00:00", greater);
  check_comparison(EXESS_TIME, "00:00:00Z", EXESS_TIME, "12:00:00", greater);

  // Date

  // Equality
  check_comparison(EXESS_DATE, "2001-02-03", EXESS_DATE, "2001-02-03", equal);
  check_comparison(EXESS_DATE, "2001-02-03Z", EXESS_DATE, "2001-02-03Z", equal);
  check_comparison(
    EXESS_DATE, "2001-02-03Z", EXESS_DATE, "2001-02-03+00:00", equal);
  check_comparison(
    EXESS_DATE, "2001-02-03Z", EXESS_DATE, "2001-02-03-00:00", equal);

  // All local
  check_comparisons(EXESS_DATE, "2001-02-03", "2001-02-04", "2001-02-05");
  check_comparisons(EXESS_DATE, "2001-02-03", "2001-03-03", "2001-04-03");
  check_comparisons(EXESS_DATE, "2001-02-03", "2002-02-03", "2003-02-03");

  // All UTC
  check_comparisons(EXESS_DATE, "2001-02-03Z", "2001-02-04Z", "2001-02-05");
  check_comparisons(EXESS_DATE, "2001-02-03Z", "2001-03-03Z", "2001-04-03");
  check_comparisons(EXESS_DATE, "2001-02-03Z", "2002-02-03Z", "2003-02-03");

  // Time zones
  check_comparisons(
    EXESS_DATE, "2001-02-03+03:00", "2001-02-03+02:00", "2001-02-03+01:00");

  // Local and UTC
  check_comparisons(EXESS_DATE, "2001-02-03Z", "2001-02-04", "2001-02-04Z");
  check_comparisons(EXESS_DATE, "2001-02-03Z", "2001-02-04", "2001-02-05Z");
  check_comparison(
    EXESS_DATE, "2001-02-03Z", EXESS_DATE, "2001-02-03", greater);
  check_comparison(EXESS_DATE, "2001-02-03", EXESS_DATE, "2001-02-03Z", less);
  check_comparison(
    EXESS_DATE, "2001-02-03Z", EXESS_DATE, "2001-02-03+01:00", greater);
  check_comparison(
    EXESS_DATE, "2001-02-03Z", EXESS_DATE, "2001-02-03-01:00", less);
  check_comparison(
    EXESS_DATE, "2001-02-03Z", EXESS_DATE, "2001-02-03+14:00", greater);

  // Binary
  check_comparisons(EXESS_HEX, "010203", "010204", "010205");
  check_comparisons(EXESS_HEX, "0102", "010204", "010205");
  check_comparisons(EXESS_HEX, "0102", "010204", "0103");
  check_comparisons(EXESS_HEX, "01", "0101", "010101");
  check_comparison(EXESS_HEX, "01", EXESS_HEX, "01", equal);
  check_comparison(EXESS_HEX, "02", EXESS_HEX, "01", greater);
  check_comparison(EXESS_HEX, "01", EXESS_HEX, "02", less);
  check_comparison(EXESS_HEX, "09", EXESS_HEX, "0102", greater);
  check_comparison(EXESS_HEX, "0102", EXESS_HEX, "010304", less);
  check_comparison(EXESS_HEX, "0101", EXESS_HEX, "01", greater);
  check_comparisons(EXESS_BASE64, "Zg==", "Zm8=", "Zm9v");
  check_comparison(EXESS_BASE64, "Zm9v", EXESS_BASE64, "Zm9v", equal);
}

int
main(void)
{
  test_read_variant();
  test_variant_string_length();
  test_write_variant();
  test_make_get();
  test_compare();

  return 0;
}
