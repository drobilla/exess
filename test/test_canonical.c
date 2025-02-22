// Copyright 2011-2025 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#undef NDEBUG

#include "write_test_utils.h"

#include <exess/exess.h>

#include <assert.h>
#include <stddef.h>
#include <string.h>

static void
check_write(const ExessDatatype datatype,
            const char* const   value,
            const ExessStatus   expected_status,
            const size_t        buf_size,
            const char* const   expected_string)
{
  char buf[EXESS_MAX_DECIMAL_LENGTH + 1] = {42};
  init_out_buf(sizeof(buf), buf);

  assert(buf_size <= sizeof(buf));

  const ExessResult r = exess_write_canonical(value, datatype, buf_size, buf);
  assert(r.status == expected_status);
  assert(r.count == strlen(buf));
  assert(!strcmp(buf, expected_string));
  assert((r.status && r.status != EXESS_EXPECTED_END) ||
         exess_write_canonical(value, datatype, 0, NULL).count == r.count);
}

static void
test_decimal(void)
{
  check_write(EXESS_DECIMAL, "", EXESS_EXPECTED_DIGIT, 1, "");
  check_write(EXESS_DECIMAL, " \f\n\r\t\v", EXESS_EXPECTED_DIGIT, 1, "");

  check_write(EXESS_DECIMAL, " -001 ", EXESS_SUCCESS, 5, "-1.0");
  check_write(EXESS_DECIMAL, " -000 ", EXESS_SUCCESS, 5, "-0.0");
  check_write(EXESS_DECIMAL, "  000 ", EXESS_SUCCESS, 4, "0.0");
  check_write(EXESS_DECIMAL, "  001 ", EXESS_SUCCESS, 4, "1.0");
  check_write(EXESS_DECIMAL, " +001 ", EXESS_SUCCESS, 4, "1.0");

  check_write(EXESS_DECIMAL, " -.123", EXESS_SUCCESS, 7, "-0.123");
  check_write(EXESS_DECIMAL, " +.123", EXESS_SUCCESS, 6, "0.123");
  check_write(EXESS_DECIMAL, " -0 ", EXESS_SUCCESS, 5, "-0.0");
  check_write(EXESS_DECIMAL, " +0 ", EXESS_SUCCESS, 4, "0.0");

  check_write(EXESS_DECIMAL, " +00.10 ", EXESS_SUCCESS, 4, "0.1");
  check_write(EXESS_DECIMAL, " +01 ", EXESS_SUCCESS, 4, "1.0");

  check_write(EXESS_DECIMAL,
              " 36893488147419103232 ",
              EXESS_SUCCESS,
              23,
              "36893488147419103232.0");

  check_write(EXESS_DECIMAL,
              " 0036893488147419103232 ",
              EXESS_SUCCESS,
              23,
              "36893488147419103232.0");

  check_write(EXESS_DECIMAL,
              " +36893488147419103232 ",
              EXESS_SUCCESS,
              23,
              "36893488147419103232.0");

  check_write(EXESS_DECIMAL,
              " -36893488147419103232 ",
              EXESS_SUCCESS,
              24,
              "-36893488147419103232.0");

  check_write(EXESS_DECIMAL,
              " +0036893488147419103232 ",
              EXESS_SUCCESS,
              23,
              "36893488147419103232.0");

  check_write(EXESS_DECIMAL,
              " +0036893488147419103232. ",
              EXESS_SUCCESS,
              23,
              "36893488147419103232.0");

  check_write(EXESS_DECIMAL,
              " +0036893488147419103232.00 ",
              EXESS_SUCCESS,
              23,
              "36893488147419103232.0");

  check_write(EXESS_DECIMAL,
              " +0036893488147419103232.12300 ",
              EXESS_SUCCESS,
              25,
              "36893488147419103232.123");

  check_write(EXESS_DECIMAL,
              " -0036893488147419103232 ",
              EXESS_SUCCESS,
              24,
              "-36893488147419103232.0");

  check_write(EXESS_DECIMAL,
              " -0036893488147419103232. ",
              EXESS_SUCCESS,
              24,
              "-36893488147419103232.0");

  check_write(EXESS_DECIMAL,
              " -0036893488147419103232.00 ",
              EXESS_SUCCESS,
              24,
              "-36893488147419103232.0");

  check_write(EXESS_DECIMAL,
              " -0036893488147419103232.12300 ",
              EXESS_SUCCESS,
              26,
              "-36893488147419103232.123");

  check_write(EXESS_DECIMAL, " -1234extra", EXESS_EXPECTED_END, 8, "-1234.0");
  check_write(EXESS_DECIMAL, " 1234extra", EXESS_EXPECTED_END, 7, "1234.0");

  check_write(EXESS_DECIMAL, " f", EXESS_EXPECTED_DIGIT, 2, "");
  check_write(EXESS_DECIMAL, "", EXESS_EXPECTED_DIGIT, 1, "");
}

static void
test_integer(void)
{
  check_write(EXESS_INTEGER, "", EXESS_EXPECTED_DIGIT, 1, "");
  check_write(EXESS_INTEGER, " \f\n\r\t\v", EXESS_EXPECTED_DIGIT, 1, "");

  // Integer

  check_write(EXESS_INTEGER, " -001 ", EXESS_SUCCESS, 3, "-1");
  check_write(EXESS_INTEGER, "  000 ", EXESS_SUCCESS, 2, "0");
  check_write(EXESS_INTEGER, "  001 ", EXESS_SUCCESS, 2, "1");
  check_write(EXESS_INTEGER, " +001 ", EXESS_SUCCESS, 2, "1");

  check_write(EXESS_INTEGER, " junk 987654321 ", EXESS_EXPECTED_DIGIT, 2, "");
  check_write(
    EXESS_INTEGER, " 987654321 junk ", EXESS_EXPECTED_END, 10, "987654321");

  check_write(EXESS_INTEGER,
              " 36893488147419103232 ",
              EXESS_SUCCESS,
              21,
              "36893488147419103232");

  check_write(EXESS_INTEGER,
              " 0036893488147419103232 ",
              EXESS_SUCCESS,
              21,
              "36893488147419103232");

  check_write(EXESS_INTEGER,
              " +36893488147419103232 ",
              EXESS_SUCCESS,
              21,
              "36893488147419103232");

  check_write(EXESS_INTEGER,
              " +0036893488147419103232 ",
              EXESS_SUCCESS,
              21,
              "36893488147419103232");

  check_write(EXESS_INTEGER,
              " -36893488147419103232 ",
              EXESS_SUCCESS,
              22,
              "-36893488147419103232");

  check_write(EXESS_INTEGER,
              " -0036893488147419103232 ",
              EXESS_SUCCESS,
              22,
              "-36893488147419103232");

  // NonPositiveInteger

  check_write(EXESS_NON_POSITIVE_INTEGER, " -001 ", EXESS_SUCCESS, 3, "-1");
  check_write(EXESS_NON_POSITIVE_INTEGER, "  000 ", EXESS_SUCCESS, 2, "0");
  check_write(EXESS_NON_POSITIVE_INTEGER, "  001 ", EXESS_BAD_VALUE, 3, "");
  check_write(EXESS_NON_POSITIVE_INTEGER, " +001 ", EXESS_BAD_VALUE, 3, "");

  check_write(EXESS_NON_POSITIVE_INTEGER,
              " -36893488147419103232 ",
              EXESS_SUCCESS,
              22,
              "-36893488147419103232");

  check_write(EXESS_NON_POSITIVE_INTEGER,
              " -0036893488147419103232 ",
              EXESS_SUCCESS,
              22,
              "-36893488147419103232");

  // NegativeInteger

  check_write(EXESS_NEGATIVE_INTEGER, " -001 ", EXESS_SUCCESS, 3, "-1");
  check_write(EXESS_NEGATIVE_INTEGER, "  000 ", EXESS_BAD_VALUE, 3, "");
  check_write(EXESS_NEGATIVE_INTEGER, "  001 ", EXESS_BAD_VALUE, 3, "");
  check_write(EXESS_NEGATIVE_INTEGER, " +001 ", EXESS_BAD_VALUE, 3, "");

  check_write(EXESS_NEGATIVE_INTEGER,
              " -36893488147419103232 ",
              EXESS_SUCCESS,
              22,
              "-36893488147419103232");

  check_write(EXESS_NEGATIVE_INTEGER,
              " -0036893488147419103232 ",
              EXESS_SUCCESS,
              22,
              "-36893488147419103232");

  // NonNegativeInteger

  check_write(EXESS_NON_NEGATIVE_INTEGER, " -001 ", EXESS_BAD_VALUE, 3, "");
  check_write(EXESS_NON_NEGATIVE_INTEGER, "  000 ", EXESS_SUCCESS, 2, "0");
  check_write(EXESS_NON_NEGATIVE_INTEGER, "  001 ", EXESS_SUCCESS, 2, "1");
  check_write(EXESS_NON_NEGATIVE_INTEGER, " +001 ", EXESS_SUCCESS, 2, "1");

  check_write(EXESS_NON_NEGATIVE_INTEGER,
              " 36893488147419103232 ",
              EXESS_SUCCESS,
              21,
              "36893488147419103232");

  check_write(EXESS_NON_NEGATIVE_INTEGER,
              " 0036893488147419103232 ",
              EXESS_SUCCESS,
              21,
              "36893488147419103232");

  // PositiveInteger

  check_write(EXESS_POSITIVE_INTEGER, " -001 ", EXESS_BAD_VALUE, 3, "");
  check_write(EXESS_POSITIVE_INTEGER, "  000 ", EXESS_BAD_VALUE, 3, "");
  check_write(EXESS_POSITIVE_INTEGER, "  001 ", EXESS_SUCCESS, 2, "1");
  check_write(EXESS_POSITIVE_INTEGER, " +001 ", EXESS_SUCCESS, 2, "1");

  check_write(EXESS_POSITIVE_INTEGER,
              " 36893488147419103232 ",
              EXESS_SUCCESS,
              21,
              "36893488147419103232");

  check_write(EXESS_POSITIVE_INTEGER,
              " 0036893488147419103232 ",
              EXESS_SUCCESS,
              21,
              "36893488147419103232");
}

static void
test_fixed_numbers(void)
{
  check_write(EXESS_DOUBLE, " +00.10 ", EXESS_SUCCESS, 7, "1.0E-1");
  check_write(EXESS_FLOAT, " +00.10 ", EXESS_SUCCESS, 14, "1.00000001E-1");
  check_write(EXESS_BOOLEAN, " 0 ", EXESS_SUCCESS, 6, "false");
  check_write(EXESS_BOOLEAN, " 1 ", EXESS_SUCCESS, 5, "true");
  check_write(EXESS_LONG, " +012 ", EXESS_SUCCESS, 3, "12");
  check_write(EXESS_INT, " +012 ", EXESS_SUCCESS, 3, "12");
  check_write(EXESS_SHORT, " +012 ", EXESS_SUCCESS, 3, "12");
  check_write(EXESS_BYTE, " +012 ", EXESS_SUCCESS, 3, "12");
  check_write(EXESS_ULONG, " 012 ", EXESS_SUCCESS, 3, "12");
  check_write(EXESS_UINT, " 012 ", EXESS_SUCCESS, 3, "12");
  check_write(EXESS_USHORT, " 012 ", EXESS_SUCCESS, 3, "12");
  check_write(EXESS_UBYTE, " 012 ", EXESS_SUCCESS, 3, "12");

  check_write(EXESS_BOOLEAN, "X", EXESS_EXPECTED_BOOLEAN, 1, "");
  check_write(EXESS_FLOAT, "X", EXESS_EXPECTED_DIGIT, 1, "");
  check_write(EXESS_DOUBLE, "X", EXESS_EXPECTED_DIGIT, 1, "");
}

static void
test_time(void)
{
  check_write(EXESS_DURATION, " P0Y6M ", EXESS_SUCCESS, 4, "P6M");
  check_write(EXESS_DURATION, " P1Y6M0D ", EXESS_SUCCESS, 6, "P1Y6M");
  check_write(EXESS_TIME, " 12:15:01+00:00 ", EXESS_SUCCESS, 10, "12:15:01Z");
  check_write(EXESS_TIME, " 24:00:00 ", EXESS_SUCCESS, 9, "00:00:00");
  check_write(EXESS_TIME, " 24:00:00Z ", EXESS_SUCCESS, 10, "00:00:00Z");
  check_write(
    EXESS_TIME, " 24:00:00+02:00 ", EXESS_SUCCESS, 15, "00:00:00+02:00");
  check_write(
    EXESS_DATE, " 02004-04-12+00:00 ", EXESS_SUCCESS, 12, "2004-04-12Z");
}

static void
test_date_time(void)
{
  // Read error
  check_write(
    EXESS_DATE_TIME, "99-01-01T12:00:00", EXESS_EXPECTED_DIGIT, 1, "");

  // Integer seconds
  check_write(EXESS_DATE_TIME,
              " 02001-02-03T04:05:06.0000 ",
              EXESS_SUCCESS,
              20,
              "2001-02-03T04:05:06");

  // Local
  check_write(EXESS_DATE_TIME,
              " 02001-02-03T04:05:06.007 ",
              EXESS_SUCCESS,
              26,
              "2001-02-03T04:05:06.007");

  // UTC
  check_write(EXESS_DATE_TIME,
              " 02001-02-03T04:46:59Z ",
              EXESS_SUCCESS,
              21,
              "2001-02-03T04:46:59Z");

  // UTC redundant negative zero form
  check_write(EXESS_DATE_TIME,
              " 02001-02-03T04:46:59-00:00 ",
              EXESS_SUCCESS,
              21,
              "2001-02-03T04:46:59Z");

  // UTC redundant positive zero form
  check_write(EXESS_DATE_TIME,
              " 02001-02-03T04:46:59+00:00 ",
              EXESS_SUCCESS,
              21,
              "2001-02-03T04:46:59Z");

  // Midnight at end of day
  check_write(EXESS_DATE_TIME,
              " 02001-02-03T24:00:00+00:00 ",
              EXESS_SUCCESS,
              21,
              "2001-02-04T00:00:00Z");

  // Midnight at end of February in a normal year
  check_write(EXESS_DATE_TIME,
              " 02001-02-28T24:00:00+00:00 ",
              EXESS_SUCCESS,
              21,
              "2001-03-01T00:00:00Z");

  // Midnight at end of the second last day of February in a leap year
  check_write(EXESS_DATE_TIME,
              " 02000-02-28T24:00:00+00:00 ",
              EXESS_SUCCESS,
              21,
              "2000-02-29T00:00:00Z");

  // Midnight at end February in a leap year
  check_write(EXESS_DATE_TIME,
              " 02000-02-29T24:00:00+00:00 ",
              EXESS_SUCCESS,
              21,
              "2000-03-01T00:00:00Z");

  // Happy new year!
  check_write(EXESS_DATE_TIME,
              " 02001-12-31T24:00:00+00:00 ",
              EXESS_SUCCESS,
              21,
              "2002-01-01T00:00:00Z");
}

static void
test_binary(void)
{
  check_write(EXESS_HEX, " D EA  D B3 3F", EXESS_SUCCESS, 9, "DEADB33F");
  check_write(EXESS_HEX, "invalid", EXESS_EXPECTED_HEX, 1, "");
  check_write(EXESS_HEX, "1A2B3", EXESS_EXPECTED_HEX, 5, "");
  check_write(EXESS_HEX, "1", EXESS_EXPECTED_HEX, 1, "");
  check_write(EXESS_HEX, "", EXESS_EXPECTED_HEX, 5, "");

  check_write(
    EXESS_BASE64, " Z\fm\n9\rv\tY\vmFy", EXESS_SUCCESS, 9, "Zm9vYmFy");
  check_write(EXESS_BASE64, "!nvalid", EXESS_EXPECTED_BASE64, 1, "");
  check_write(EXESS_BASE64, "Z", EXESS_EXPECTED_BASE64, 1, "");
  check_write(EXESS_BASE64, "Zm", EXESS_EXPECTED_BASE64, 2, "");
  check_write(EXESS_BASE64, "Zm9", EXESS_EXPECTED_BASE64, 3, "");
  check_write(EXESS_BASE64, "", EXESS_EXPECTED_BASE64, 5, "");
}

int
main(void)
{
  check_write(EXESS_NOTHING, "?", EXESS_UNSUPPORTED, 1, "");

  test_decimal();
  test_integer();
  test_fixed_numbers();
  test_time();
  test_date_time();
  test_binary();

  return 0;
}
