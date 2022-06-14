// Copyright 2021 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#undef NDEBUG

#include "float_test_data.h"

#include "exess/exess.h"

#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#define MAX_VALUE_SIZE 32u

static const ExessDuration duration = {14,
                                       (3 * 24 * 60 * 60) + (4 * 60 * 60) +
                                         (5 * 60) + 6,
                                       0};

static const ExessDateTime datetime = {2001, 2, 3, false, 4, 5, 6, 0};
static const ExessTime     time     = {EXESS_UTC, 1, 2, 3, 0};
static const ExessDate     date     = {2001, 2, 3, EXESS_UTC};

static void
test_size(void)
{
  assert(sizeof(ExessValue) == EXESS_MAX_VALUE_SIZE);

  assert(sizeof(ExessDuration) <= EXESS_MAX_VALUE_SIZE);
  assert(sizeof(ExessDateTime) <= EXESS_MAX_VALUE_SIZE);
  assert(sizeof(ExessTime) <= EXESS_MAX_VALUE_SIZE);
  assert(sizeof(ExessDate) <= EXESS_MAX_VALUE_SIZE);
}

static void
check_read(void*               value,
           const ExessDatatype datatype,
           const char* const   string,
           const ExessStatus   expected_status,
           const size_t        expected_read_count)
{
  const ExessVariableResult r =
    exess_read_value(datatype, MAX_VALUE_SIZE, value, string);

  assert(r.status == expected_status);
  assert(r.read_count == expected_read_count);
  assert(r.status || r.write_count == exess_value_size(datatype));
}

static void
test_read_value(void)
{
  {
    int64_t                   a_long = 0;
    const ExessVariableResult bad =
      exess_read_value(EXESS_LONG, 7, &a_long, "1");
    assert(bad.status == EXESS_NO_SPACE);
  }
  {
    bool a_bool = false;
    check_read(&a_bool, EXESS_BOOLEAN, "true", EXESS_SUCCESS, 4);
    assert(a_bool);
  }
  {
    double a_double = (double)NAN;

    check_read(&a_double, EXESS_DECIMAL, "1.2", EXESS_SUCCESS, 3);
    assert(double_matches(a_double, 1.2));

    check_read(&a_double, EXESS_DOUBLE, "3.4", EXESS_SUCCESS, 3);
    assert(double_matches(a_double, 3.4));
  }
  {
    float a_float = NAN;
    check_read(&a_float, EXESS_FLOAT, "5.6", EXESS_SUCCESS, 3);
    assert(float_matches(a_float, 5.6F));
  }
  {
    int64_t a_long = 0;

    check_read(&a_long, EXESS_NOTHING, "42", EXESS_UNSUPPORTED, 0);

    check_read(&a_long, EXESS_INTEGER, "7", EXESS_SUCCESS, 1);
    assert(a_long == 7);

    check_read(
      &a_long, EXESS_NON_POSITIVE_INTEGER, "f", EXESS_EXPECTED_DIGIT, 0);
    check_read(&a_long, EXESS_NON_POSITIVE_INTEGER, "1", EXESS_OUT_OF_RANGE, 1);
    check_read(&a_long, EXESS_NON_POSITIVE_INTEGER, "-8", EXESS_SUCCESS, 2);
    assert(a_long == -8);

    check_read(&a_long, EXESS_NEGATIVE_INTEGER, "f", EXESS_EXPECTED_DIGIT, 0);
    check_read(&a_long, EXESS_NEGATIVE_INTEGER, "1", EXESS_OUT_OF_RANGE, 1);
    check_read(&a_long, EXESS_NEGATIVE_INTEGER, "-9", EXESS_SUCCESS, 2);
    assert(a_long == -9);

    check_read(&a_long, EXESS_LONG, "10", EXESS_SUCCESS, 2);
    assert(a_long == 10);
  }
  {
    int32_t a_int = 0;
    check_read(&a_int, EXESS_INT, "11", EXESS_SUCCESS, 2);
    assert(a_int == 11);
  }
  {
    int16_t a_short = 0;
    check_read(&a_short, EXESS_SHORT, "12", EXESS_SUCCESS, 2);
    assert(a_short == 12);
  }
  {
    int8_t a_byte = 0;
    check_read(&a_byte, EXESS_BYTE, "13", EXESS_SUCCESS, 2);
    assert(a_byte == 13);
  }
  {
    uint64_t a_ulong = 0U;
    check_read(&a_ulong, EXESS_ULONG, "14", EXESS_SUCCESS, 2);
    assert(a_ulong == 14U);
  }
  {
    uint32_t a_uint = 0U;
    check_read(&a_uint, EXESS_UINT, "15", EXESS_SUCCESS, 2);
    assert(a_uint == 15U);
  }
  {
    uint16_t a_ushort = 0U;
    check_read(&a_ushort, EXESS_USHORT, "16", EXESS_SUCCESS, 2);
    assert(a_ushort == 16U);
  }
  {
    uint8_t a_ubyte = 0U;
    check_read(&a_ubyte, EXESS_UBYTE, "17", EXESS_SUCCESS, 2);
    assert(a_ubyte == 17U);
  }
  {
    uint64_t a_ulong = 0U;
    check_read(&a_ulong, EXESS_POSITIVE_INTEGER, "-1", EXESS_EXPECTED_DIGIT, 0);
    check_read(&a_ulong, EXESS_POSITIVE_INTEGER, "0", EXESS_OUT_OF_RANGE, 1);
    check_read(&a_ulong, EXESS_POSITIVE_INTEGER, "18", EXESS_SUCCESS, 2);
    assert(a_ulong == 18U);
  }
  {
    ExessDateTime a_datetime = datetime;
    check_read(
      &a_datetime, EXESS_DATETIME, "2001-01-02T12:15:01.25", EXESS_SUCCESS, 22);

    assert(a_datetime.year == 2001);
    assert(a_datetime.month == 1);
    assert(a_datetime.day == 2);
    assert(a_datetime.hour == 12);
    assert(a_datetime.minute == 15);
    assert(a_datetime.second == 1);
    assert(a_datetime.nanosecond == 250000000);
  }
  {
    ExessDate a_date = date;
    check_read(&a_date, EXESS_DATE, "2001-01-02", EXESS_SUCCESS, 10);

    assert(a_date.year == 2001);
    assert(a_date.month == 1);
    assert(a_date.day == 2);
  }
  {
    ExessTime a_time = time;
    check_read(&a_time, EXESS_TIME, "12:15:01.25", EXESS_SUCCESS, 11);

    assert(a_time.hour == 12);
    assert(a_time.minute == 15);
    assert(a_time.second == 1);
    assert(a_time.nanosecond == 250000000);
  }
  {
    char                      a_blob[] = {0, 0, 0};
    const ExessVariableResult r =
      exess_read_value(EXESS_HEX, sizeof(a_blob), a_blob, "666F6F");

    assert(!r.status);
    assert(r.read_count == 6);
    assert(r.write_count == sizeof(a_blob));
    assert(!strncmp(a_blob, "foo", sizeof(a_blob)));
  }
  {
    char                      a_blob[] = {0, 0, 0};
    const ExessVariableResult r =
      exess_read_value(EXESS_BASE64, sizeof(a_blob), a_blob, "Zm9v");

    assert(!r.status);
    assert(r.read_count == 4);
    assert(r.write_count == sizeof(a_blob));
    assert(!strncmp(a_blob, "foo", sizeof(a_blob)));
  }
}

static void
check_write(const ExessDatatype datatype,
            const size_t        value_size,
            const void*         value,
            const ExessStatus   expected_status,
            const size_t        buf_size,
            const char* const   expected_string)
{
  char buf[EXESS_MAX_DECIMAL_LENGTH + 1] = {42};

  assert(buf_size <= sizeof(buf));

  const ExessResult r =
    exess_write_value(datatype, value_size, value, buf_size, buf);

  assert(r.status == expected_status);

  if (buf_size > 0) {
    assert(r.count == strlen(buf));
    assert(!strcmp(buf, expected_string));
  }
}

static void
test_write_value(void)
{
  uint8_t a_blob[] = {'f', 'o', 'o'};

  const bool          a_bool     = true;
  const double        a_decimal  = 1.2;
  const double        a_double   = 3.4;
  const float         a_float    = 5.6F;
  const int64_t       a_long     = 7;
  const int32_t       a_int      = 8;
  const int16_t       a_short    = 9;
  const int8_t        a_byte     = 10;
  const uint64_t      a_ulong    = 11U;
  const uint32_t      a_uint     = 12U;
  const uint16_t      a_ushort   = 13U;
  const uint8_t       a_ubyte    = 14U;
  const ExessDuration a_duration = duration;
  const ExessDateTime a_datetime = datetime;
  const ExessTime     a_time     = time;
  const ExessDate     a_date     = date;

  check_write(EXESS_NOTHING, sizeof(a_bool), &a_bool, EXESS_BAD_VALUE, 0, "");

  check_write(EXESS_BOOLEAN, sizeof(a_bool), &a_bool, EXESS_SUCCESS, 5, "true");

  check_write(
    EXESS_DECIMAL, sizeof(a_decimal), &a_decimal, EXESS_SUCCESS, 4, "1.2");
  check_write(
    EXESS_DOUBLE, sizeof(a_double), &a_double, EXESS_SUCCESS, 6, "3.4E0");
  check_write(
    EXESS_FLOAT, sizeof(a_float), &a_float, EXESS_SUCCESS, 12, "5.5999999E0");

  check_write(EXESS_LONG, sizeof(a_long), &a_long, EXESS_SUCCESS, 2, "7");
  check_write(EXESS_INT, sizeof(a_int), &a_int, EXESS_SUCCESS, 2, "8");
  check_write(EXESS_SHORT, sizeof(a_short), &a_short, EXESS_SUCCESS, 2, "9");
  check_write(EXESS_BYTE, sizeof(a_byte), &a_byte, EXESS_SUCCESS, 3, "10");

  check_write(EXESS_ULONG, sizeof(a_ulong), &a_ulong, EXESS_SUCCESS, 3, "11");
  check_write(EXESS_UINT, sizeof(a_uint), &a_uint, EXESS_SUCCESS, 3, "12");
  check_write(
    EXESS_USHORT, sizeof(a_ushort), &a_ushort, EXESS_SUCCESS, 3, "13");
  check_write(EXESS_UBYTE, sizeof(a_ubyte), &a_ubyte, EXESS_SUCCESS, 3, "14");

  check_write(EXESS_DURATION,
              sizeof(a_duration),
              &a_duration,
              EXESS_SUCCESS,
              15,
              "P1Y2M3DT4H5M6S");

  check_write(EXESS_DATETIME,
              sizeof(a_datetime),
              &a_datetime,
              EXESS_SUCCESS,
              40,
              "2001-02-03T04:05:06");

  check_write(
    EXESS_TIME, sizeof(a_time), &a_time, EXESS_SUCCESS, 40, "01:02:03Z");
  check_write(
    EXESS_DATE, sizeof(a_date), &a_date, EXESS_SUCCESS, 40, "2001-02-03Z");

  check_write(EXESS_HEX, sizeof(a_blob), a_blob, EXESS_SUCCESS, 7, "666F6F");
  check_write(EXESS_BASE64, sizeof(a_blob), a_blob, EXESS_SUCCESS, 5, "Zm9v");
}

int
main(void)
{
  test_size();
  test_read_value();
  test_write_value();

  return 0;
}
