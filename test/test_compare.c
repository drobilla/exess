// Copyright 2011-2021 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#undef NDEBUG

#include "exess/exess.h"

#include <assert.h>
#include <stdbool.h>

static bool
less(const int cmp)
{
  return cmp < 0;
}

static bool
equal(const int cmp)
{
  return cmp == 0;
}

static bool
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
  ExessValue lhs = {false};
  ExessValue rhs = {false};

  const ExessVariableResult l =
    exess_read_value(lhs_datatype, sizeof(lhs), &lhs, lhs_string);

  const ExessVariableResult r =
    exess_read_value(rhs_datatype, sizeof(rhs), &rhs, rhs_string);

  assert(lhs_datatype == EXESS_NOTHING || !l.status);
  assert(rhs_datatype == EXESS_NOTHING || !r.status);
  assert(pred(exess_compare_value(
    lhs_datatype, l.write_count, &lhs, rhs_datatype, r.write_count, &rhs)));
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
  test_compare();

  return 0;
}
