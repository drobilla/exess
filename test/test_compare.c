// Copyright 2011-2025 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#undef NDEBUG

#include <exess/exess.h>

#include <assert.h>
#include <stdbool.h>

typedef bool (*ComparisonPredicate)(int);

static bool
less(const int cmp)
{
  return cmp == EXESS_ORDER_STRICTLY_LESS;
}

static bool
maybe_less(const int cmp)
{
  return cmp == EXESS_ORDER_MAYBE_LESS;
}

static bool
equal(const int cmp)
{
  return cmp == EXESS_ORDER_EQUAL;
}

static bool
maybe_greater(const int cmp)
{
  return cmp == EXESS_ORDER_MAYBE_GREATER;
}

static bool
greater(const int cmp)
{
  return cmp == EXESS_ORDER_STRICTLY_GREATER;
}

static void
check_cross_type_comparison(const ExessDatatype       lhs_datatype,
                            const char* const         lhs_string,
                            const ExessDatatype       rhs_datatype,
                            const char* const         rhs_string,
                            const ComparisonPredicate pred)
{
  ExessValue lhs = {false};
  ExessValue rhs = {false};

  const ExessVariableResult l =
    exess_read_value(lhs_datatype, sizeof(lhs), &lhs, lhs_string);

  const ExessVariableResult r =
    exess_read_value(rhs_datatype, sizeof(rhs), &rhs, rhs_string);

  assert(lhs_datatype == EXESS_NOTHING || !l.status);
  assert(rhs_datatype == EXESS_NOTHING || !r.status);

  const ExessOrder order = exess_compare_value(
    lhs_datatype, l.write_count, &lhs, rhs_datatype, r.write_count, &rhs);

  assert(pred(order));

  // NOLINTNEXTLINE(readability-suspicious-call-argument)
  const ExessOrder inv_order = exess_compare_value(
    rhs_datatype, r.write_count, &rhs, lhs_datatype, l.write_count, &lhs);

  if (order == EXESS_ORDER_STRICTLY_LESS) {
    assert(inv_order == EXESS_ORDER_STRICTLY_GREATER);
  } else if (order == EXESS_ORDER_MAYBE_LESS) {
    assert(inv_order == EXESS_ORDER_MAYBE_GREATER);
  } else if (order == EXESS_ORDER_EQUAL) {
    assert(inv_order == EXESS_ORDER_EQUAL);
  } else if (order == EXESS_ORDER_MAYBE_GREATER) {
    assert(inv_order == EXESS_ORDER_MAYBE_LESS);
  } else if (order == EXESS_ORDER_STRICTLY_GREATER) {
    assert(inv_order == EXESS_ORDER_STRICTLY_LESS);
  }
}

static void
check_comparison(const ExessDatatype       datatype,
                 const char* const         lhs_string,
                 const char* const         rhs_string,
                 const ComparisonPredicate pred)
{
  check_cross_type_comparison(datatype, lhs_string, datatype, rhs_string, pred);
}

static void
check_comparisons_partial(const ExessDatatype datatype,
                          const char* const   low,
                          const char* const   mid,
                          const char* const   high)
{
  check_comparison(datatype, low, mid, maybe_less);
  check_comparison(datatype, mid, mid, equal);
  check_comparison(datatype, high, mid, maybe_greater);
}

static void
check_comparisons(const ExessDatatype datatype,
                  const char* const   low,
                  const char* const   mid,
                  const char* const   high)
{
  check_comparison(datatype, low, mid, less);
  check_comparison(datatype, mid, mid, equal);
  check_comparison(datatype, high, mid, greater);
}

static void
test_compare(void)
{
  // Cross-type and nothing
  check_cross_type_comparison(EXESS_NOTHING, "", EXESS_NOTHING, "", equal);
  check_cross_type_comparison(EXESS_NOTHING, "", EXESS_INT, "0", maybe_less);
  check_cross_type_comparison(EXESS_INT, "0", EXESS_NOTHING, "", maybe_greater);
  check_cross_type_comparison(
    EXESS_BOOLEAN, "true", EXESS_INT, "0", maybe_less);
  check_cross_type_comparison(
    EXESS_TIME, "12:00:00", EXESS_INT, "42", maybe_greater);

  // All boolean cases
  check_comparison(EXESS_BOOLEAN, "false", "true", less);
  check_comparison(EXESS_BOOLEAN, "false", "1", less);
  check_comparison(EXESS_BOOLEAN, "0", "true", less);
  check_comparison(EXESS_BOOLEAN, "0", "1", less);
  check_comparison(EXESS_BOOLEAN, "false", "false", equal);
  check_comparison(EXESS_BOOLEAN, "false", "0", equal);
  check_comparison(EXESS_BOOLEAN, "0", "0", equal);
  check_comparison(EXESS_BOOLEAN, "0", "false", equal);
  check_comparison(EXESS_BOOLEAN, "true", "true", equal);
  check_comparison(EXESS_BOOLEAN, "true", "1", equal);
  check_comparison(EXESS_BOOLEAN, "1", "1", equal);
  check_comparison(EXESS_BOOLEAN, "1", "true", equal);
  check_comparison(EXESS_BOOLEAN, "true", "false", greater);
  check_comparison(EXESS_BOOLEAN, "true", "0", greater);
  check_comparison(EXESS_BOOLEAN, "1", "0", greater);

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

  // Trivially comparable
  check_comparisons(EXESS_DURATION, "-P9Y", "P0Y", "P9Y");
  check_comparisons(EXESS_DURATION, "-P9M", "P0M", "P9M");
  check_comparisons(EXESS_DURATION, "-P9D", "P0D", "P9D");
  check_comparisons(EXESS_DURATION, "-PT9H", "PT0H", "PT9H");
  check_comparisons(EXESS_DURATION, "-PT9M", "PT0M", "PT9M");
  check_comparisons(EXESS_DURATION, "-PT9S", "PT0S", "PT9S");
  check_comparisons(EXESS_DURATION, "-PT0.9S", "PT0.0S", "PT0.9S");

  // Months and years
  check_comparison(EXESS_DURATION, "P11M", "P1Y", less);
  check_comparison(EXESS_DURATION, "P12M", "P1Y", equal);
  check_comparison(EXESS_DURATION, "P13M", "P1Y", greater);

  // Months and days
  check_comparison(EXESS_DURATION, "P27D", "P1M", less);
  check_comparison(EXESS_DURATION, "P31D", "P1M", maybe_less);
  check_comparison(EXESS_DURATION, "P32D", "P1M", greater);
  check_comparison(EXESS_DURATION, "P1M", "P28D", maybe_greater);
  check_comparison(EXESS_DURATION, "P330D", "P6M", maybe_less);
  check_comparison(EXESS_DURATION, "P4M", "P4M28D", maybe_less);
  check_comparison(EXESS_DURATION, "P4M", "P4M31D", maybe_less);
  check_comparison(EXESS_DURATION, "P4M", "P4M32D", maybe_less);
  check_comparison(EXESS_DURATION, "P3M29D", "P4M", less);
  check_comparison(EXESS_DURATION, "P3M31D", "P4M", maybe_less);
  check_comparison(EXESS_DURATION, "P4M27D", "P4M", maybe_greater);

  // Months and seconds
  check_comparison(EXESS_DURATION, "P1MT0.000000001S", "P1M", greater);
  check_comparison(EXESS_DURATION, "P1M", "P1MT0.000000001S", less);

  // Days, seconds, and months
  check_comparison(EXESS_DURATION, "P3M30DT86400S", "P4M", maybe_less);
  check_comparison(EXESS_DURATION, "P3M30DT86400.1S", "P4M", greater);
  check_comparison(EXESS_DURATION, "P3M30DT86401S", "P4M", greater);
  check_comparison(EXESS_DURATION, "P3M31DT0.000000001S", "P4M", greater);
  check_comparison(EXESS_DURATION, "P3M32D", "P4M", greater);

  // Seconds and days
  check_comparison(EXESS_DURATION, "PT86399S", "P1D", less);
  check_comparison(EXESS_DURATION, "PT86400S", "P1D", equal);
  check_comparison(EXESS_DURATION, "PT86401S", "P1D", greater);

  // Seconds and months
  check_comparison(EXESS_DURATION, "PT2332800S", "P1M", less);
  check_comparison(EXESS_DURATION, "PT2678400S", "P1M", maybe_less);
  check_comparison(EXESS_DURATION, "PT2678401S", "P1M", greater);

  // Seconds and years
  check_comparison(EXESS_DURATION, "PT31535999S", "P1Y", less);
  check_comparison(EXESS_DURATION, "PT31536000S", "P1Y", maybe_less);
  check_comparison(EXESS_DURATION, "PT31622400S", "P1Y", maybe_less);
  check_comparison(EXESS_DURATION, "PT31622401S", "P1Y", greater);

  // DateTime

  // Equality
  check_comparison(
    EXESS_DATE_TIME, "2001-02-03T12:13:14", "2001-02-03T12:13:14", equal);
  check_comparison(
    EXESS_DATE_TIME, "2001-02-03T12:13:14Z", "2001-02-03T12:13:14Z", equal);

  // All local
  check_comparisons(EXESS_DATE_TIME,
                    "2001-02-03T12:13:14.15",
                    "2001-02-03T12:13:14.16",
                    "2001-02-03T12:13:14.17");
  check_comparisons(EXESS_DATE_TIME,
                    "2001-02-03T12:13:14",
                    "2001-02-03T12:13:15",
                    "2001-02-03T12:13:16");
  check_comparisons(EXESS_DATE_TIME,
                    "2001-02-03T12:13:14",
                    "2001-02-03T12:14:14",
                    "2001-02-03T12:15:14");
  check_comparisons(EXESS_DATE_TIME,
                    "2001-02-03T12:13:14",
                    "2001-02-03T13:13:14",
                    "2001-02-03T14:13:14");
  check_comparisons(EXESS_DATE_TIME,
                    "2001-02-03T12:13:14",
                    "2001-02-04T12:13:14",
                    "2001-02-05T12:13:14");
  check_comparisons(EXESS_DATE_TIME,
                    "2001-02-03T12:13:14",
                    "2001-03-03T12:13:14",
                    "2001-04-03T12:13:14");
  check_comparisons(EXESS_DATE_TIME,
                    "2001-02-03T12:13:14",
                    "2002-02-03T12:13:14",
                    "2003-02-03T12:13:14");

  // All UTC
  check_comparisons(EXESS_DATE_TIME,
                    "2001-02-03T12:13:14.15Z",
                    "2001-02-03T12:13:14.16Z",
                    "2001-02-03T12:13:14.17Z");
  check_comparisons(EXESS_DATE_TIME,
                    "2001-02-03T12:13:14Z",
                    "2001-02-03T12:13:15Z",
                    "2001-02-03T12:13:16Z");
  check_comparisons(EXESS_DATE_TIME,
                    "2001-02-03T12:13:14Z",
                    "2001-02-03T12:14:14Z",
                    "2001-02-03T12:15:14Z");
  check_comparisons(EXESS_DATE_TIME,
                    "2001-02-03T12:13:14Z",
                    "2001-02-03T13:13:14Z",
                    "2001-02-03T14:13:14Z");
  check_comparisons(EXESS_DATE_TIME,
                    "2001-02-03T12:13:14Z",
                    "2001-02-04T12:13:14Z",
                    "2001-02-05T12:13:14Z");
  check_comparisons(EXESS_DATE_TIME,
                    "2001-02-03T12:13:14Z",
                    "2001-03-03T12:13:14Z",
                    "2001-04-03T12:13:14Z");
  check_comparisons(EXESS_DATE_TIME,
                    "2001-02-03T12:13:14Z",
                    "2002-02-03T12:13:14Z",
                    "2003-02-03T12:13:14Z");

  // Local and UTC comparable
  check_comparisons(EXESS_DATE_TIME,
                    "2001-02-03T12:13:14Z",
                    "2001-02-04T02:13:15",
                    "2001-02-04T16:13:15.000000001Z");
  check_comparisons(EXESS_DATE_TIME,
                    "2001-02-03T12:13:14Z",
                    "2001-02-04T12:13:14",
                    "2001-02-05T12:13:14Z");
  check_comparisons(EXESS_DATE_TIME,
                    "2001-02-03T12:13:14",
                    "2001-02-04T02:13:15Z",
                    "2001-02-04T16:13:16");

  // Local and UTC incomparable, "same" time
  check_comparison(EXESS_DATE_TIME,
                   "2001-02-03T12:13:14Z",
                   "2001-02-03T12:13:14",
                   maybe_greater);

  // Local and UTC incomparable, exactly -14 hours apart
  check_comparison(EXESS_DATE_TIME,
                   "2001-02-04T16:13:15Z",
                   "2001-02-04T02:13:15",
                   maybe_greater);

  // Local and UTC incomparable, exactly +14 hours apart
  check_comparison(
    EXESS_DATE_TIME, "2001-02-04T02:13:15", "2001-02-04T16:13:15Z", maybe_less);

  // Local and UTC comparable, just over +14 hours apart
  check_comparison(EXESS_DATE_TIME,
                   "2001-02-04T02:13:15",
                   "2001-02-04T16:13:15.000000001Z",
                   less);

  // Local and UTC comparable (example from spec)
  check_comparison(
    EXESS_DATE_TIME, "2000-01-15T12:00:00", "2000-01-16T12:00:00Z", less);

  // Local and UTC incomparable (examples from spec, local is first here)
  check_comparison(
    EXESS_DATE_TIME, "2000-01-01T12:00:00", "1999-12-31T23:00:00Z", maybe_less);
  check_comparison(
    EXESS_DATE_TIME, "2000-01-16T12:00:00", "2000-01-16T12:00:00Z", maybe_less);
  check_comparison(
    EXESS_DATE_TIME, "2000-01-16T00:00:00", "2000-01-16T12:00:00Z", maybe_less);

  // Time

  // Equality
  check_comparison(EXESS_TIME, "12:13:14", "12:13:14", equal);
  check_comparison(EXESS_TIME, "12:13:14Z", "12:13:14Z", equal);
  check_comparison(EXESS_TIME, "12:13:14", "12:13:14", equal);
  check_comparison(EXESS_TIME, "12:13:14+00:00", "12:13:14+00:00", equal);
  check_comparison(EXESS_TIME, "12:13:14", "12:13:14", equal);
  check_comparison(EXESS_TIME, "12:13:14-00:00", "12:13:14-00:00", equal);

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

  // Local and UTC incomparable
  check_comparison(EXESS_TIME, "12:13:14", "02:13:15Z", maybe_less);
  check_comparison(EXESS_TIME, "02:13:15Z", "12:13:14", maybe_greater);

  // Local and UTC comparable
  check_comparison(EXESS_TIME, "01:00:00Z", "15:00:01", less);
  check_comparison(EXESS_TIME, "15:00:01", "01:00:00Z", greater);
  check_comparison(EXESS_TIME, "02:13:15Z", "16:13:16", less);
  check_comparison(EXESS_TIME, "16:13:16", "02:13:15Z", greater);

  // Local and UTC incomparable (local is first here)
  check_comparison(EXESS_TIME, "12:00:00", "12:00:00Z", maybe_less);
  check_comparison(EXESS_TIME, "00:00:00", "12:00:00Z", maybe_less);
  check_comparison(EXESS_TIME, "12:00:00Z", "12:00:00", maybe_greater);
  check_comparison(EXESS_TIME, "00:00:00Z", "12:00:00", maybe_greater);
  check_comparison(EXESS_TIME, "15:00:00Z", "01:00:00", maybe_greater);
  check_comparison(EXESS_TIME, "01:00:00", "15:00:00Z", maybe_less);

  // Date

  // Equality
  check_comparison(EXESS_DATE, "2001-02-03", "2001-02-03", equal);
  check_comparison(EXESS_DATE, "2001-02-03Z", "2001-02-03Z", equal);
  check_comparison(EXESS_DATE, "2001-02-03Z", "2001-02-03+00:00", equal);
  check_comparison(EXESS_DATE, "2001-02-03Z", "2001-02-03-00:00", equal);

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
  check_comparison(EXESS_DATE, "2001-02-03Z", "2001-02-04", less);
  check_comparison(EXESS_DATE, "2001-02-04", "2001-02-04Z", maybe_less);
  check_comparison(EXESS_DATE, "2001-02-03Z", "2001-02-04", less);
  check_comparison(EXESS_DATE, "2001-02-04", "2001-02-05Z", less);
  check_comparison(EXESS_DATE, "2001-02-03Z", "2001-02-03", maybe_greater);
  check_comparison(EXESS_DATE, "2001-02-03", "2001-02-03Z", maybe_less);
  check_comparison(EXESS_DATE, "2001-02-03Z", "2001-02-03+01:00", greater);
  check_comparison(EXESS_DATE, "2001-02-03Z", "2001-02-03-01:00", less);
  check_comparison(EXESS_DATE, "2001-02-03Z", "2001-02-03+14:00", greater);

  // Binary

  check_comparisons_partial(EXESS_HEX, "010203", "010204", "010205");
  check_comparisons_partial(EXESS_HEX, "0102", "010204", "010205");
  check_comparisons_partial(EXESS_HEX, "0102", "010204", "0103");
  check_comparisons_partial(EXESS_HEX, "01", "0101", "010101");
  check_comparison(EXESS_HEX, "01", "01", equal);
  check_comparison(EXESS_HEX, "02", "01", maybe_greater);
  check_comparison(EXESS_HEX, "01", "02", maybe_less);
  check_comparison(EXESS_HEX, "09", "0102", maybe_greater);
  check_comparison(EXESS_HEX, "0102", "010304", maybe_less);
  check_comparison(EXESS_HEX, "0101", "01", maybe_greater);
  check_comparisons_partial(EXESS_BASE64, "Zg==", "Zm8=", "Zm9v");
  check_comparison(EXESS_BASE64, "Zm9v", "Zm9v", equal);
}

int
main(void)
{
  test_compare();

  return 0;
}
