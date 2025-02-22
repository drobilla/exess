// Copyright 2011-2025 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#undef NDEBUG

#include "write_test_utils.h"

#include <exess/exess.h>

#include <assert.h>
#include <stddef.h>
#include <string.h>

static void
check(const ExessDatatype datatype,
      const size_t        expected_read_count,
      const char* const   value,
      const ExessStatus   expected_status,
      const size_t        expected_write_count,
      const char* const   expected_string)
{
  char buf[EXESS_MAX_DECIMAL_LENGTH + 1] = {42};
  init_out_buf(sizeof(buf), buf);

  assert(expected_write_count < sizeof(buf));

  const ExessVariableResult r =
    exess_write_canonical(value, datatype, expected_write_count + 1, buf);

  assert(r.status == expected_status);
  assert(r.read_count == expected_read_count);
  assert(r.write_count == expected_write_count);
  assert(expected_write_count == strlen(buf));
  assert(!strcmp(buf, expected_string));
  assert((r.status && r.status != EXESS_EXPECTED_END) ||
         exess_write_canonical(value, datatype, 0, NULL).write_count ==
           r.write_count);
}

static void
test_decimal(void)
{
  check(EXESS_DECIMAL, 0, "", EXESS_EXPECTED_DIGIT, 0, "");
  check(EXESS_DECIMAL, 6, " \f\n\r\t\v", EXESS_EXPECTED_DIGIT, 0, "");

  check(EXESS_DECIMAL, 5, " -001 ", EXESS_SUCCESS, 4, "-1.0");
  check(EXESS_DECIMAL, 5, " -000 ", EXESS_SUCCESS, 4, "-0.0");
  check(EXESS_DECIMAL, 5, "  000 ", EXESS_SUCCESS, 3, "0.0");
  check(EXESS_DECIMAL, 5, "  001 ", EXESS_SUCCESS, 3, "1.0");
  check(EXESS_DECIMAL, 5, " +001 ", EXESS_SUCCESS, 3, "1.0");

  check(EXESS_DECIMAL, 6, " -.123", EXESS_SUCCESS, 6, "-0.123");
  check(EXESS_DECIMAL, 6, " +.123", EXESS_SUCCESS, 5, "0.123");
  check(EXESS_DECIMAL, 3, " -0 ", EXESS_SUCCESS, 4, "-0.0");
  check(EXESS_DECIMAL, 3, " +0 ", EXESS_SUCCESS, 3, "0.0");

  check(EXESS_DECIMAL, 7, " +00.10 ", EXESS_SUCCESS, 3, "0.1");
  check(EXESS_DECIMAL, 4, " +01 ", EXESS_SUCCESS, 3, "1.0");

  check(EXESS_DECIMAL,
        21,
        " 36893488147419103232 ",
        EXESS_SUCCESS,
        22,
        "36893488147419103232.0");

  check(EXESS_DECIMAL,
        23,
        " 0036893488147419103232 ",
        EXESS_SUCCESS,
        22,
        "36893488147419103232.0");

  check(EXESS_DECIMAL,
        22,
        " +36893488147419103232 ",
        EXESS_SUCCESS,
        22,
        "36893488147419103232.0");

  check(EXESS_DECIMAL,
        22,
        " -36893488147419103232 ",
        EXESS_SUCCESS,
        23,
        "-36893488147419103232.0");

  check(EXESS_DECIMAL,
        24,
        " +0036893488147419103232 ",
        EXESS_SUCCESS,
        22,
        "36893488147419103232.0");

  check(EXESS_DECIMAL,
        25,
        " +0036893488147419103232. ",
        EXESS_SUCCESS,
        22,
        "36893488147419103232.0");

  check(EXESS_DECIMAL,
        27,
        " +0036893488147419103232.00 ",
        EXESS_SUCCESS,
        22,
        "36893488147419103232.0");

  check(EXESS_DECIMAL,
        30,
        " +0036893488147419103232.12300 ",
        EXESS_SUCCESS,
        24,
        "36893488147419103232.123");

  check(EXESS_DECIMAL,
        24,
        " -0036893488147419103232 ",
        EXESS_SUCCESS,
        23,
        "-36893488147419103232.0");

  check(EXESS_DECIMAL,
        25,
        " -0036893488147419103232. ",
        EXESS_SUCCESS,
        23,
        "-36893488147419103232.0");

  check(EXESS_DECIMAL,
        27,
        " -0036893488147419103232.00 ",
        EXESS_SUCCESS,
        23,
        "-36893488147419103232.0");

  check(EXESS_DECIMAL,
        30,
        " -0036893488147419103232.12300 ",
        EXESS_SUCCESS,
        25,
        "-36893488147419103232.123");

  check(EXESS_DECIMAL, 6, " -1234extra", EXESS_EXPECTED_END, 0, "");
  check(EXESS_DECIMAL, 5, " 1234extra", EXESS_EXPECTED_END, 0, "");

  check(EXESS_DECIMAL, 1, " f", EXESS_EXPECTED_DIGIT, 0, "");
  check(EXESS_DECIMAL, 0, "", EXESS_EXPECTED_DIGIT, 0, "");
}

static void
test_integer(void)
{
  check(EXESS_INTEGER, 0, "", EXESS_EXPECTED_DIGIT, 0, "");
  check(EXESS_INTEGER, 6, " \f\n\r\t\v", EXESS_EXPECTED_DIGIT, 0, "");

  // Integer

  check(EXESS_INTEGER, 5, " -001 ", EXESS_SUCCESS, 2, "-1");
  check(EXESS_INTEGER, 5, "  000 ", EXESS_SUCCESS, 1, "0");
  check(EXESS_INTEGER, 5, "  001 ", EXESS_SUCCESS, 1, "1");
  check(EXESS_INTEGER, 5, " +001 ", EXESS_SUCCESS, 1, "1");

  check(EXESS_INTEGER, 1, " junk 987654321 ", EXESS_EXPECTED_DIGIT, 0, "");
  check(EXESS_INTEGER, 10, " 987654321 junk ", EXESS_SUCCESS, 9, "987654321");

  check(EXESS_INTEGER,
        21,
        " 36893488147419103232 ",
        EXESS_SUCCESS,
        20,
        "36893488147419103232");

  check(EXESS_INTEGER,
        23,
        " 0036893488147419103232 ",
        EXESS_SUCCESS,
        20,
        "36893488147419103232");

  check(EXESS_INTEGER,
        22,
        " +36893488147419103232 ",
        EXESS_SUCCESS,
        20,
        "36893488147419103232");

  check(EXESS_INTEGER,
        24,
        " +0036893488147419103232 ",
        EXESS_SUCCESS,
        20,
        "36893488147419103232");

  check(EXESS_INTEGER,
        22,
        " -36893488147419103232 ",
        EXESS_SUCCESS,
        21,
        "-36893488147419103232");

  check(EXESS_INTEGER,
        24,
        " -0036893488147419103232 ",
        EXESS_SUCCESS,
        21,
        "-36893488147419103232");

  // NonPositiveInteger

  check(EXESS_NON_POSITIVE_INTEGER, 5, " -001 ", EXESS_SUCCESS, 2, "-1");
  check(EXESS_NON_POSITIVE_INTEGER, 5, "  000 ", EXESS_SUCCESS, 1, "0");
  check(EXESS_NON_POSITIVE_INTEGER, 4, "  001 ", EXESS_BAD_VALUE, 0, "");
  check(EXESS_NON_POSITIVE_INTEGER, 1, " +001 ", EXESS_BAD_VALUE, 0, "");

  check(EXESS_NON_POSITIVE_INTEGER,
        22,
        " -36893488147419103232 ",
        EXESS_SUCCESS,
        21,
        "-36893488147419103232");

  check(EXESS_NON_POSITIVE_INTEGER,
        24,
        " -0036893488147419103232 ",
        EXESS_SUCCESS,
        21,
        "-36893488147419103232");

  // NegativeInteger

  check(EXESS_NEGATIVE_INTEGER, 5, " -001 ", EXESS_SUCCESS, 2, "-1");
  check(EXESS_NEGATIVE_INTEGER, 2, "  000 ", EXESS_BAD_VALUE, 0, "");
  check(EXESS_NEGATIVE_INTEGER, 2, "  001 ", EXESS_BAD_VALUE, 0, "");
  check(EXESS_NEGATIVE_INTEGER, 1, " +001 ", EXESS_BAD_VALUE, 0, "");

  check(EXESS_NEGATIVE_INTEGER,
        22,
        " -36893488147419103232 ",
        EXESS_SUCCESS,
        21,
        "-36893488147419103232");

  check(EXESS_NEGATIVE_INTEGER,
        24,
        " -0036893488147419103232 ",
        EXESS_SUCCESS,
        21,
        "-36893488147419103232");

  // NonNegativeInteger

  check(EXESS_NON_NEGATIVE_INTEGER, 1, " -001 ", EXESS_BAD_VALUE, 0, "");
  check(EXESS_NON_NEGATIVE_INTEGER, 5, "  000 ", EXESS_SUCCESS, 1, "0");
  check(EXESS_NON_NEGATIVE_INTEGER, 5, "  001 ", EXESS_SUCCESS, 1, "1");
  check(EXESS_NON_NEGATIVE_INTEGER, 5, " +001 ", EXESS_SUCCESS, 1, "1");

  check(EXESS_NON_NEGATIVE_INTEGER,
        21,
        " 36893488147419103232 ",
        EXESS_SUCCESS,
        20,
        "36893488147419103232");

  check(EXESS_NON_NEGATIVE_INTEGER,
        23,
        " 0036893488147419103232 ",
        EXESS_SUCCESS,
        20,
        "36893488147419103232");

  // PositiveInteger

  check(EXESS_POSITIVE_INTEGER, 1, " -001 ", EXESS_BAD_VALUE, 0, "");
  check(EXESS_POSITIVE_INTEGER, 2, "  000 ", EXESS_BAD_VALUE, 0, "");
  check(EXESS_POSITIVE_INTEGER, 5, "  001 ", EXESS_SUCCESS, 1, "1");
  check(EXESS_POSITIVE_INTEGER, 5, " +001 ", EXESS_SUCCESS, 1, "1");

  check(EXESS_POSITIVE_INTEGER,
        21,
        " 36893488147419103232 ",
        EXESS_SUCCESS,
        20,
        "36893488147419103232");

  check(EXESS_POSITIVE_INTEGER,
        23,
        " 0036893488147419103232 ",
        EXESS_SUCCESS,
        20,
        "36893488147419103232");
}

static void
test_fixed_numbers(void)
{
  check(EXESS_DOUBLE, 7, " +00.10 ", EXESS_SUCCESS, 6, "1.0E-1");
  check(EXESS_FLOAT, 7, " +00.10 ", EXESS_SUCCESS, 13, "1.00000001E-1");
  check(EXESS_BOOLEAN, 2, " 0 ", EXESS_SUCCESS, 5, "false");
  check(EXESS_BOOLEAN, 2, " 1 ", EXESS_SUCCESS, 4, "true");
  check(EXESS_LONG, 5, " +012 ", EXESS_SUCCESS, 2, "12");
  check(EXESS_INT, 5, " +012 ", EXESS_SUCCESS, 2, "12");
  check(EXESS_SHORT, 5, " +012 ", EXESS_SUCCESS, 2, "12");
  check(EXESS_BYTE, 5, " +012 ", EXESS_SUCCESS, 2, "12");
  check(EXESS_ULONG, 4, " 012 ", EXESS_SUCCESS, 2, "12");
  check(EXESS_UINT, 4, " 012 ", EXESS_SUCCESS, 2, "12");
  check(EXESS_USHORT, 4, " 012 ", EXESS_SUCCESS, 2, "12");
  check(EXESS_UBYTE, 4, " 012 ", EXESS_SUCCESS, 2, "12");

  check(EXESS_BOOLEAN, 0, "X", EXESS_EXPECTED_BOOLEAN, 0, "");
  check(EXESS_FLOAT, 0, "X", EXESS_EXPECTED_DIGIT, 0, "");
  check(EXESS_DOUBLE, 0, "X", EXESS_EXPECTED_DIGIT, 0, "");
}

static void
test_time(void)
{
  check(EXESS_DURATION, 6, " P0Y6M ", EXESS_SUCCESS, 3, "P6M");
  check(EXESS_DURATION, 8, " P1Y6M0D ", EXESS_SUCCESS, 5, "P1Y6M");
  check(EXESS_TIME, 15, " 12:15:01+00:00 ", EXESS_SUCCESS, 9, "12:15:01Z");
  check(EXESS_TIME, 9, " 24:00:00 ", EXESS_SUCCESS, 8, "00:00:00");
  check(EXESS_TIME, 10, " 24:00:00Z ", EXESS_SUCCESS, 9, "00:00:00Z");
  check(
    EXESS_TIME, 15, " 24:00:00+02:00 ", EXESS_SUCCESS, 14, "00:00:00+02:00");
  check(
    EXESS_DATE, 18, " 02004-04-12+00:00 ", EXESS_SUCCESS, 11, "2004-04-12Z");
}

static void
test_date_time(void)
{
  // Read error
  check(EXESS_DATE_TIME, 2, "99-01-01T12:00:00", EXESS_EXPECTED_DIGIT, 0, "");

  // Integer seconds
  check(EXESS_DATE_TIME,
        26,
        " 02001-02-03T04:05:06.0000 ",
        EXESS_SUCCESS,
        19,
        "2001-02-03T04:05:06");

  // Local
  check(EXESS_DATE_TIME,
        25,
        " 02001-02-03T04:05:06.007 ",
        EXESS_SUCCESS,
        23,
        "2001-02-03T04:05:06.007");

  // UTC
  check(EXESS_DATE_TIME,
        22,
        " 02001-02-03T04:46:59Z ",
        EXESS_SUCCESS,
        20,
        "2001-02-03T04:46:59Z");

  // UTC redundant negative zero form
  check(EXESS_DATE_TIME,
        27,
        " 02001-02-03T04:46:59-00:00 ",
        EXESS_SUCCESS,
        20,
        "2001-02-03T04:46:59Z");

  // UTC redundant positive zero form
  check(EXESS_DATE_TIME,
        27,
        " 02001-02-03T04:46:59+00:00 ",
        EXESS_SUCCESS,
        20,
        "2001-02-03T04:46:59Z");

  // Midnight at end of day
  check(EXESS_DATE_TIME,
        27,
        " 02001-02-03T24:00:00+00:00 ",
        EXESS_SUCCESS,
        20,
        "2001-02-04T00:00:00Z");

  // Midnight at end of February in a normal year
  check(EXESS_DATE_TIME,
        27,
        " 02001-02-28T24:00:00+00:00 ",
        EXESS_SUCCESS,
        20,
        "2001-03-01T00:00:00Z");

  // Midnight at end of the second last day of February in a leap year
  check(EXESS_DATE_TIME,
        27,
        " 02000-02-28T24:00:00+00:00 ",
        EXESS_SUCCESS,
        20,
        "2000-02-29T00:00:00Z");

  // Midnight at end February in a leap year
  check(EXESS_DATE_TIME,
        27,
        " 02000-02-29T24:00:00+00:00 ",
        EXESS_SUCCESS,
        20,
        "2000-03-01T00:00:00Z");

  // Happy new year!
  check(EXESS_DATE_TIME,
        27,
        " 02001-12-31T24:00:00+00:00 ",
        EXESS_SUCCESS,
        20,
        "2002-01-01T00:00:00Z");
}

static void
test_binary(void)
{
  check(EXESS_HEX, 14, " D EA  D B3 3F", EXESS_SUCCESS, 8, "DEADB33F");
  check(EXESS_HEX, 0, "invalid", EXESS_EXPECTED_HEX, 0, "");
  check(EXESS_HEX, 5, "1A2B3", EXESS_EXPECTED_HEX, 0, "");
  check(EXESS_HEX, 1, "1", EXESS_EXPECTED_HEX, 0, "");
  check(EXESS_HEX, 0, "", EXESS_EXPECTED_HEX, 0, "");

  check(EXESS_BASE64, 14, " Z\fm\n9\rv\tY\vmFy", EXESS_SUCCESS, 8, "Zm9vYmFy");
  check(EXESS_BASE64, 0, "!nvalid", EXESS_EXPECTED_BASE64, 0, "");
  check(EXESS_BASE64, 1, "Z", EXESS_EXPECTED_BASE64, 0, "");
  check(EXESS_BASE64, 2, "Zm", EXESS_EXPECTED_BASE64, 0, "");
  check(EXESS_BASE64, 3, "Zm9", EXESS_EXPECTED_BASE64, 0, "");
  check(EXESS_BASE64, 0, "", EXESS_EXPECTED_BASE64, 0, "");
}

int
main(void)
{
  check(EXESS_NOTHING, 0, "?", EXESS_UNSUPPORTED, 0, "");

  test_decimal();
  test_integer();
  test_fixed_numbers();
  test_time();
  test_date_time();
  test_binary();

  return 0;
}
