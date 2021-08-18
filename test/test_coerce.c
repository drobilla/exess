// Copyright 2011-2021 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#undef NDEBUG

#include "exess/exess.h"

#include <assert.h>
#include <string.h>

static const char* const min_long_str  = "-9223372036854775808";
static const char* const max_long_str  = "9223372036854775807";
static const char* const max_ulong_str = "18446744073709551615";

static void
check_from_to(const ExessDatatype from_datatype,
              const char* const   from_string,
              const ExessDatatype to_datatype,
              const char* const   to_string)
{
  char value_buf[4] = {0, 0, 0, 0};
  char buf[328]     = {42};

  // Read original value
  ExessVariant value = {EXESS_HEX, {.as_blob = {sizeof(value_buf), value_buf}}};
  assert(!exess_read_variant(&value, from_datatype, from_string).status);

  // Coerce to target datatype
  const ExessVariant coerced = exess_coerce(value, to_datatype, EXESS_LOSSLESS);
  assert(coerced.datatype == to_datatype);

  // Write coerced value and check string against expectation
  assert(!exess_write_variant(coerced, sizeof(buf), buf).status);
  assert(!strcmp(buf, to_string));

  // Coerce the value back to the original type
  const ExessVariant tripped =
    exess_coerce(coerced, from_datatype, EXESS_LOSSLESS);
  assert(tripped.datatype == from_datatype);

  // Write round-tripped value and check string against the original
  assert(!exess_write_variant(tripped, sizeof(buf), buf).status);
  assert(!strcmp(buf, from_string));
}

static void
check_one_way(const ExessDatatype      from_datatype,
              const char* const        from_string,
              const ExessCoercionFlags coercions,
              const ExessDatatype      to_datatype,
              const char* const        to_string)
{
  char buf[328] = {42};

  // Read original value
  ExessVariant value = {EXESS_NOTHING, {EXESS_SUCCESS}};
  assert(!exess_read_variant(&value, from_datatype, from_string).status);

  // Coerce to target datatype
  ExessVariant coerced = exess_coerce(value, to_datatype, coercions);
  assert(coerced.datatype == to_datatype);

  // Write coerced value and check string against expectation
  assert(!exess_write_variant(coerced, sizeof(buf), buf).status);
  assert(!strcmp(buf, to_string));
}

static void
check_failure(const ExessDatatype from_datatype,
              const char* const   from_string,
              const ExessDatatype to_datatype,
              const ExessStatus   expected_status)
{
  // Read original value
  ExessVariant value = {EXESS_NOTHING, {EXESS_SUCCESS}};
  assert(!exess_read_variant(&value, from_datatype, from_string).status);

  // Try to coerce to target datatype
  const ExessVariant coerced = exess_coerce(value, to_datatype, EXESS_LOSSLESS);
  assert(coerced.datatype == EXESS_NOTHING);
  assert(exess_get_status(&coerced) == expected_status);
}

static void
test_unknown(void)
{
  ExessVariant long_value    = exess_make_long(1);
  ExessVariant ulong_value   = exess_make_ulong(1u);
  ExessVariant unknown_value = exess_make_nothing(EXESS_SUCCESS);

  assert(exess_coerce(unknown_value, EXESS_LONG, EXESS_LOSSLESS).datatype ==
         EXESS_NOTHING);

  assert(exess_coerce(unknown_value, EXESS_ULONG, EXESS_LOSSLESS).datatype ==
         EXESS_NOTHING);

  assert(exess_coerce(long_value, EXESS_NOTHING, EXESS_LOSSLESS).datatype ==
         EXESS_NOTHING);

  assert(exess_coerce(ulong_value, EXESS_NOTHING, EXESS_LOSSLESS).datatype ==
         EXESS_NOTHING);
}

static void
test_decimal(void)
{
  check_from_to(EXESS_DOUBLE, "1.2E3", EXESS_DECIMAL, "1200.0");
  check_from_to(EXESS_LONG, "1200", EXESS_DECIMAL, "1200.0");

  check_one_way(EXESS_FLOAT, "1.2E3", 0, EXESS_DECIMAL, "1200.0");
  check_one_way(EXESS_FLOAT, "1.5E0", 0, EXESS_DOUBLE, "1.5E0");

  check_failure(
    EXESS_DECIMAL, "1.5", EXESS_FLOAT, EXESS_WOULD_REDUCE_PRECISION);

  check_failure(
    EXESS_DOUBLE, "1.5E0", EXESS_FLOAT, EXESS_WOULD_REDUCE_PRECISION);

  check_failure(
    EXESS_DECIMAL, "9223372036854775808.0", EXESS_LONG, EXESS_OUT_OF_RANGE);

  check_failure(
    EXESS_LONG, "9007199254740992", EXESS_DECIMAL, EXESS_OUT_OF_RANGE);

  check_failure(EXESS_DOUBLE, "1E38", EXESS_LONG, EXESS_OUT_OF_RANGE);

  check_failure(EXESS_FLOAT, "1E308", EXESS_LONG, EXESS_OUT_OF_RANGE);
}

static void
test_boolean(void)
{
  // Exactly from 0 or 1, lossy from 0 or non-zero
  check_from_to(EXESS_BOOLEAN, "false", EXESS_FLOAT, "0.0E0");
  check_from_to(EXESS_BOOLEAN, "true", EXESS_FLOAT, "1.0E0");
  check_from_to(EXESS_BOOLEAN, "false", EXESS_DOUBLE, "0.0E0");
  check_from_to(EXESS_BOOLEAN, "true", EXESS_DOUBLE, "1.0E0");
  check_from_to(EXESS_BOOLEAN, "false", EXESS_LONG, "0");
  check_from_to(EXESS_BOOLEAN, "true", EXESS_LONG, "1");
  check_failure(EXESS_LONG, "-1", EXESS_BOOLEAN, EXESS_WOULD_TRUNCATE);
  check_failure(EXESS_LONG, "2", EXESS_BOOLEAN, EXESS_WOULD_TRUNCATE);
  check_one_way(EXESS_LONG, "42", EXESS_TRUNCATE, EXESS_BOOLEAN, "true");
  check_one_way(EXESS_LONG, "-1", EXESS_TRUNCATE, EXESS_BOOLEAN, "true");
  check_from_to(EXESS_BOOLEAN, "false", EXESS_ULONG, "0");
  check_from_to(EXESS_BOOLEAN, "true", EXESS_ULONG, "1");
  check_failure(EXESS_LONG, "2", EXESS_BOOLEAN, EXESS_WOULD_TRUNCATE);
  check_one_way(EXESS_ULONG, "42", EXESS_TRUNCATE, EXESS_BOOLEAN, "true");

  // Not convertible to any time types
  check_failure(EXESS_BOOLEAN, "true", EXESS_DURATION, EXESS_UNSUPPORTED);
  check_failure(EXESS_BOOLEAN, "true", EXESS_DATETIME, EXESS_UNSUPPORTED);
  check_failure(EXESS_BOOLEAN, "true", EXESS_TIME, EXESS_UNSUPPORTED);
  check_failure(EXESS_BOOLEAN, "true", EXESS_DATE, EXESS_UNSUPPORTED);
}

static void
test_long(void)
{
  // Truncating boolean conversion
  check_one_way(EXESS_LONG, "42", EXESS_TRUNCATE, EXESS_BOOLEAN, "true");
  check_one_way(EXESS_LONG, "-1", EXESS_TRUNCATE, EXESS_BOOLEAN, "true");

  // All smaller integer types
  check_from_to(EXESS_LONG, "-2147483648", EXESS_INT, "-2147483648");
  check_failure(EXESS_LONG, "-2147483649", EXESS_INT, EXESS_OUT_OF_RANGE);
  check_from_to(EXESS_LONG, "-32768", EXESS_SHORT, "-32768");
  check_failure(EXESS_LONG, "-32769", EXESS_SHORT, EXESS_OUT_OF_RANGE);
  check_from_to(EXESS_LONG, "-128", EXESS_BYTE, "-128");
  check_failure(EXESS_LONG, "-129", EXESS_BYTE, EXESS_OUT_OF_RANGE);

  // Positive values to/from all unsigned types
  check_from_to(EXESS_LONG, max_long_str, EXESS_ULONG, max_long_str);
  check_failure(EXESS_LONG, "-1", EXESS_ULONG, EXESS_OUT_OF_RANGE);
  check_from_to(EXESS_LONG, "4294967295", EXESS_UINT, "4294967295");
  check_failure(EXESS_LONG, "-1", EXESS_UINT, EXESS_OUT_OF_RANGE);
  check_from_to(EXESS_LONG, "65535", EXESS_USHORT, "65535");
  check_failure(EXESS_LONG, "-1", EXESS_USHORT, EXESS_OUT_OF_RANGE);
  check_from_to(EXESS_LONG, "255", EXESS_UBYTE, "255");
  check_failure(EXESS_LONG, "-1", EXESS_UBYTE, EXESS_OUT_OF_RANGE);

  // Any value to/from integer
  check_from_to(EXESS_LONG, min_long_str, EXESS_INTEGER, min_long_str);
  check_from_to(EXESS_LONG, max_long_str, EXESS_INTEGER, max_long_str);

  // Non-positive values to/from nonPositiveInteger
  check_from_to(
    EXESS_LONG, min_long_str, EXESS_NON_POSITIVE_INTEGER, min_long_str);
  check_from_to(EXESS_LONG, "0", EXESS_NON_POSITIVE_INTEGER, "0");
  check_failure(
    EXESS_LONG, "1", EXESS_NON_POSITIVE_INTEGER, EXESS_OUT_OF_RANGE);

  // Negative values to/from negativeInteger
  check_from_to(EXESS_LONG, min_long_str, EXESS_NEGATIVE_INTEGER, min_long_str);
  check_from_to(EXESS_LONG, "-1", EXESS_NEGATIVE_INTEGER, "-1");
  check_failure(EXESS_LONG, "0", EXESS_NEGATIVE_INTEGER, EXESS_OUT_OF_RANGE);

  // Non-negative values to/from nonNegativeInteger
  check_failure(
    EXESS_LONG, "-1", EXESS_NON_NEGATIVE_INTEGER, EXESS_OUT_OF_RANGE);
  check_from_to(EXESS_LONG, "0", EXESS_NON_NEGATIVE_INTEGER, "0");
  check_from_to(
    EXESS_LONG, max_long_str, EXESS_NON_NEGATIVE_INTEGER, max_long_str);

  // Positive values to/from positiveInteger
  check_failure(EXESS_LONG, "-1", EXESS_POSITIVE_INTEGER, EXESS_OUT_OF_RANGE);
  check_failure(EXESS_LONG, "0", EXESS_POSITIVE_INTEGER, EXESS_OUT_OF_RANGE);
  check_from_to(EXESS_LONG, max_long_str, EXESS_POSITIVE_INTEGER, max_long_str);
  check_failure(EXESS_POSITIVE_INTEGER,
                "9223372036854775808",
                EXESS_LONG,
                EXESS_OUT_OF_RANGE);

  // Float
  check_failure(EXESS_FLOAT, "1.5", EXESS_LONG, EXESS_WOULD_ROUND);
  check_from_to(EXESS_LONG, "-16777215", EXESS_FLOAT, "-1.6777215E7");
  check_failure(EXESS_LONG, "-16777216", EXESS_FLOAT, EXESS_OUT_OF_RANGE);
  check_failure(EXESS_FLOAT, "-16777216", EXESS_LONG, EXESS_OUT_OF_RANGE);
  check_from_to(EXESS_LONG, "16777215", EXESS_FLOAT, "1.6777215E7");
  check_failure(EXESS_LONG, "16777216", EXESS_FLOAT, EXESS_OUT_OF_RANGE);
  check_failure(EXESS_FLOAT, "16777216", EXESS_LONG, EXESS_OUT_OF_RANGE);
  check_one_way(EXESS_FLOAT, "1.0", EXESS_LOSSLESS, EXESS_LONG, "1");
  check_one_way(EXESS_FLOAT, "1.5", EXESS_ROUND, EXESS_LONG, "2");
  check_one_way(EXESS_FLOAT, "2.5", EXESS_ROUND, EXESS_LONG, "2");
  check_one_way(EXESS_FLOAT, "3.5", EXESS_ROUND, EXESS_LONG, "4");

  // Double
  check_failure(EXESS_DOUBLE, "1.5", EXESS_LONG, EXESS_WOULD_ROUND);
  check_from_to(
    EXESS_LONG, "-9007199254740991", EXESS_DOUBLE, "-9.007199254740991E15");
  check_failure(
    EXESS_LONG, "-9007199254740992", EXESS_DOUBLE, EXESS_OUT_OF_RANGE);
  check_failure(
    EXESS_DOUBLE, "-9007199254740992", EXESS_LONG, EXESS_OUT_OF_RANGE);
  check_from_to(
    EXESS_LONG, "9007199254740991", EXESS_DOUBLE, "9.007199254740991E15");
  check_failure(
    EXESS_LONG, "9007199254740992", EXESS_DOUBLE, EXESS_OUT_OF_RANGE);
  check_failure(
    EXESS_DOUBLE, "9007199254740992", EXESS_LONG, EXESS_OUT_OF_RANGE);
  check_one_way(EXESS_DOUBLE, "1.0", EXESS_LOSSLESS, EXESS_LONG, "1");
  check_one_way(EXESS_DOUBLE, "1.5", EXESS_ROUND, EXESS_LONG, "2");
  check_one_way(EXESS_DOUBLE, "2.5", EXESS_ROUND, EXESS_LONG, "2");
  check_one_way(EXESS_DOUBLE, "3.5", EXESS_ROUND, EXESS_LONG, "4");
}

static void
test_ulong(void)
{
  ExessVariant unknown = {EXESS_NOTHING, {EXESS_SUCCESS}};

  assert(exess_coerce(unknown, EXESS_ULONG, EXESS_LOSSLESS).datatype ==
         EXESS_NOTHING);

  // Truncating boolean conversion
  check_one_way(EXESS_ULONG, "42", EXESS_TRUNCATE, EXESS_BOOLEAN, "true");

  // All integer types
  check_from_to(EXESS_ULONG, max_long_str, EXESS_LONG, max_long_str);
  check_failure(EXESS_ULONG, max_ulong_str, EXESS_LONG, EXESS_OUT_OF_RANGE);
  check_from_to(EXESS_ULONG, "2147483647", EXESS_INT, "2147483647");
  check_failure(EXESS_ULONG, "2147483648", EXESS_INT, EXESS_OUT_OF_RANGE);
  check_failure(EXESS_INT, "-1", EXESS_ULONG, EXESS_OUT_OF_RANGE);
  check_from_to(EXESS_ULONG, "32767", EXESS_SHORT, "32767");
  check_failure(EXESS_ULONG, "32768", EXESS_SHORT, EXESS_OUT_OF_RANGE);
  check_failure(EXESS_SHORT, "-1", EXESS_ULONG, EXESS_OUT_OF_RANGE);
  check_from_to(EXESS_ULONG, "127", EXESS_BYTE, "127");
  check_failure(EXESS_ULONG, "128", EXESS_BYTE, EXESS_OUT_OF_RANGE);
  check_failure(EXESS_BYTE, "-1", EXESS_ULONG, EXESS_OUT_OF_RANGE);

  // All unsigned types
  check_from_to(EXESS_ULONG, "4294967295", EXESS_UINT, "4294967295");
  check_failure(EXESS_ULONG, "4294967296", EXESS_UINT, EXESS_OUT_OF_RANGE);
  check_from_to(EXESS_ULONG, "65535", EXESS_USHORT, "65535");
  check_failure(EXESS_ULONG, "65536", EXESS_USHORT, EXESS_OUT_OF_RANGE);
  check_from_to(EXESS_ULONG, "255", EXESS_UBYTE, "255");
  check_failure(EXESS_ULONG, "256", EXESS_UBYTE, EXESS_OUT_OF_RANGE);

  // Limits
  check_from_to(EXESS_ULONG, "0", EXESS_INTEGER, "0");
  check_failure(
    EXESS_ULONG, "9223372036854775808", EXESS_INTEGER, EXESS_OUT_OF_RANGE);

  // Only zero to/from nonPositiveInteger
  check_from_to(EXESS_ULONG, "0", EXESS_NON_POSITIVE_INTEGER, "0");
  check_failure(
    EXESS_ULONG, "1", EXESS_NON_POSITIVE_INTEGER, EXESS_OUT_OF_RANGE);

  // Not convertible to/from negativeInteger
  check_failure(EXESS_ULONG, "0", EXESS_NEGATIVE_INTEGER, EXESS_OUT_OF_RANGE);
  check_failure(EXESS_ULONG, "1", EXESS_NEGATIVE_INTEGER, EXESS_OUT_OF_RANGE);

  // Any value to/from nonNegativeInteger
  check_from_to(EXESS_ULONG, "0", EXESS_NON_NEGATIVE_INTEGER, "0");
  check_from_to(
    EXESS_ULONG, max_ulong_str, EXESS_NON_NEGATIVE_INTEGER, max_ulong_str);

  // Positive values to/from positiveInteger
  check_failure(EXESS_ULONG, "0", EXESS_POSITIVE_INTEGER, EXESS_OUT_OF_RANGE);
  check_from_to(EXESS_ULONG, "1", EXESS_POSITIVE_INTEGER, "1");

  // Float
  check_failure(EXESS_FLOAT, "-1", EXESS_ULONG, EXESS_OUT_OF_RANGE);
  check_failure(EXESS_FLOAT, "1.5", EXESS_ULONG, EXESS_WOULD_ROUND);
  check_from_to(EXESS_ULONG, "0", EXESS_FLOAT, "0.0E0");
  check_from_to(EXESS_ULONG, "16777215", EXESS_FLOAT, "1.6777215E7");
  check_failure(EXESS_ULONG, "16777216", EXESS_FLOAT, EXESS_OUT_OF_RANGE);
  check_failure(EXESS_FLOAT, "16777216", EXESS_ULONG, EXESS_OUT_OF_RANGE);
  check_one_way(EXESS_FLOAT, "1.0", EXESS_LOSSLESS, EXESS_ULONG, "1");
  check_one_way(EXESS_FLOAT, "1.5", EXESS_ROUND, EXESS_ULONG, "2");
  check_one_way(EXESS_FLOAT, "2.5", EXESS_ROUND, EXESS_ULONG, "2");
  check_one_way(EXESS_FLOAT, "3.5", EXESS_ROUND, EXESS_ULONG, "4");

  // Double
  check_failure(EXESS_DOUBLE, "-1", EXESS_ULONG, EXESS_OUT_OF_RANGE);
  check_failure(EXESS_DOUBLE, "1.5", EXESS_ULONG, EXESS_WOULD_ROUND);
  check_from_to(EXESS_ULONG, "0", EXESS_DOUBLE, "0.0E0");
  check_from_to(
    EXESS_ULONG, "9007199254740991", EXESS_DOUBLE, "9.007199254740991E15");
  check_failure(
    EXESS_ULONG, "9007199254740992", EXESS_DOUBLE, EXESS_OUT_OF_RANGE);
  check_failure(
    EXESS_DOUBLE, "9007199254740992", EXESS_ULONG, EXESS_OUT_OF_RANGE);
  check_one_way(EXESS_DOUBLE, "1.0", EXESS_LOSSLESS, EXESS_ULONG, "1");
  check_one_way(EXESS_DOUBLE, "1.5", EXESS_ROUND, EXESS_ULONG, "2");
  check_one_way(EXESS_DOUBLE, "2.5", EXESS_ROUND, EXESS_ULONG, "2");
  check_one_way(EXESS_DOUBLE, "3.5", EXESS_ROUND, EXESS_ULONG, "4");
}

static void
test_large_integers(void)
{
  check_failure(EXESS_TIME, "00:00:00", EXESS_INTEGER, EXESS_UNSUPPORTED);
  check_failure(
    EXESS_TIME, "00:00:00", EXESS_NON_POSITIVE_INTEGER, EXESS_UNSUPPORTED);
  check_failure(
    EXESS_TIME, "00:00:00", EXESS_NEGATIVE_INTEGER, EXESS_UNSUPPORTED);
  check_failure(
    EXESS_TIME, "00:00:00", EXESS_NON_NEGATIVE_INTEGER, EXESS_UNSUPPORTED);
  check_failure(
    EXESS_TIME, "00:00:00", EXESS_POSITIVE_INTEGER, EXESS_UNSUPPORTED);
}

static void
test_lossy(void)
{
  check_one_way(EXESS_DOUBLE,
                "1.0000000000001",
                EXESS_REDUCE_PRECISION,
                EXESS_FLOAT,
                "1.0E0");

  check_failure(
    EXESS_DOUBLE, "1.0000000000001", EXESS_FLOAT, EXESS_WOULD_REDUCE_PRECISION);

  check_one_way(EXESS_FLOAT, "1.5", EXESS_LOSSLESS, EXESS_DOUBLE, "1.5E0");
}

static void
test_date_time(void)
{
  check_failure(
    EXESS_DATETIME, "2001-02-03T04:05:06", EXESS_TIME, EXESS_WOULD_TRUNCATE);

  check_one_way(EXESS_DATETIME,
                "2001-02-03T04:05:06",
                EXESS_TRUNCATE,
                EXESS_TIME,
                "04:05:06");

  check_one_way(EXESS_DATETIME,
                "2001-02-03T04:05:06Z",
                EXESS_TRUNCATE,
                EXESS_TIME,
                "04:05:06Z");

  check_failure(
    EXESS_DATETIME, "2001-02-03T04:05:06", EXESS_DATE, EXESS_WOULD_TRUNCATE);

  check_one_way(EXESS_DATETIME,
                "2001-02-03T04:05:06",
                EXESS_TRUNCATE,
                EXESS_DATE,
                "2001-02-03");

  check_one_way(EXESS_DATETIME,
                "2001-02-03T04:05:06Z",
                EXESS_TRUNCATE,
                EXESS_DATE,
                "2001-02-03Z");
}

static void
test_number_to_time(void)
{
  check_failure(EXESS_BOOLEAN, "true", EXESS_DURATION, EXESS_UNSUPPORTED);
  check_failure(EXESS_BOOLEAN, "true", EXESS_DATETIME, EXESS_UNSUPPORTED);
  check_failure(EXESS_BOOLEAN, "true", EXESS_TIME, EXESS_UNSUPPORTED);
  check_failure(EXESS_BOOLEAN, "true", EXESS_DATE, EXESS_UNSUPPORTED);

  check_failure(EXESS_LONG, "1", EXESS_DURATION, EXESS_UNSUPPORTED);
  check_failure(EXESS_LONG, "1", EXESS_DATETIME, EXESS_UNSUPPORTED);
  check_failure(EXESS_LONG, "1", EXESS_TIME, EXESS_UNSUPPORTED);
  check_failure(EXESS_LONG, "1", EXESS_DATE, EXESS_UNSUPPORTED);

  check_failure(EXESS_INT, "1", EXESS_DURATION, EXESS_UNSUPPORTED);
  check_failure(EXESS_INT, "1", EXESS_DATETIME, EXESS_UNSUPPORTED);
  check_failure(EXESS_INT, "1", EXESS_TIME, EXESS_UNSUPPORTED);
  check_failure(EXESS_INT, "1", EXESS_DATE, EXESS_UNSUPPORTED);

  check_failure(EXESS_SHORT, "1", EXESS_DURATION, EXESS_UNSUPPORTED);
  check_failure(EXESS_SHORT, "1", EXESS_DATETIME, EXESS_UNSUPPORTED);
  check_failure(EXESS_SHORT, "1", EXESS_TIME, EXESS_UNSUPPORTED);
  check_failure(EXESS_SHORT, "1", EXESS_DATE, EXESS_UNSUPPORTED);

  check_failure(EXESS_BYTE, "1", EXESS_DURATION, EXESS_UNSUPPORTED);
  check_failure(EXESS_BYTE, "1", EXESS_DATETIME, EXESS_UNSUPPORTED);
  check_failure(EXESS_BYTE, "1", EXESS_TIME, EXESS_UNSUPPORTED);
  check_failure(EXESS_BYTE, "1", EXESS_DATE, EXESS_UNSUPPORTED);

  check_failure(EXESS_ULONG, "1", EXESS_DURATION, EXESS_UNSUPPORTED);
  check_failure(EXESS_ULONG, "1", EXESS_DATETIME, EXESS_UNSUPPORTED);
  check_failure(EXESS_ULONG, "1", EXESS_TIME, EXESS_UNSUPPORTED);
  check_failure(EXESS_ULONG, "1", EXESS_DATE, EXESS_UNSUPPORTED);

  check_failure(EXESS_UINT, "1", EXESS_DURATION, EXESS_UNSUPPORTED);
  check_failure(EXESS_UINT, "1", EXESS_DATETIME, EXESS_UNSUPPORTED);
  check_failure(EXESS_UINT, "1", EXESS_TIME, EXESS_UNSUPPORTED);
  check_failure(EXESS_UINT, "1", EXESS_DATE, EXESS_UNSUPPORTED);

  check_failure(EXESS_USHORT, "1", EXESS_DURATION, EXESS_UNSUPPORTED);
  check_failure(EXESS_USHORT, "1", EXESS_DATETIME, EXESS_UNSUPPORTED);
  check_failure(EXESS_USHORT, "1", EXESS_TIME, EXESS_UNSUPPORTED);
  check_failure(EXESS_USHORT, "1", EXESS_DATE, EXESS_UNSUPPORTED);

  check_failure(EXESS_UBYTE, "1", EXESS_DURATION, EXESS_UNSUPPORTED);
  check_failure(EXESS_UBYTE, "1", EXESS_DATETIME, EXESS_UNSUPPORTED);
  check_failure(EXESS_UBYTE, "1", EXESS_TIME, EXESS_UNSUPPORTED);
  check_failure(EXESS_UBYTE, "1", EXESS_DATE, EXESS_UNSUPPORTED);
}

static void
test_time_to_number(void)
{
  static const char* const duration_str = "P1Y";
  static const char* const datetime_str = "2001-02-03T04:05:06";
  static const char* const time_str     = "04:05:06";
  static const char* const date_str     = "2001-02-03";

  check_failure(EXESS_DURATION, duration_str, EXESS_BOOLEAN, EXESS_UNSUPPORTED);
  check_failure(EXESS_DATETIME, datetime_str, EXESS_BOOLEAN, EXESS_UNSUPPORTED);
  check_failure(EXESS_TIME, time_str, EXESS_BOOLEAN, EXESS_UNSUPPORTED);
  check_failure(EXESS_DATE, date_str, EXESS_BOOLEAN, EXESS_UNSUPPORTED);

  check_failure(EXESS_DURATION, duration_str, EXESS_INT, EXESS_UNSUPPORTED);
  check_failure(EXESS_DATETIME, datetime_str, EXESS_INT, EXESS_UNSUPPORTED);
  check_failure(EXESS_TIME, time_str, EXESS_INT, EXESS_UNSUPPORTED);
  check_failure(EXESS_DATE, date_str, EXESS_INT, EXESS_UNSUPPORTED);

  check_failure(EXESS_DURATION, duration_str, EXESS_SHORT, EXESS_UNSUPPORTED);
  check_failure(EXESS_DATETIME, datetime_str, EXESS_SHORT, EXESS_UNSUPPORTED);
  check_failure(EXESS_TIME, time_str, EXESS_SHORT, EXESS_UNSUPPORTED);
  check_failure(EXESS_DATE, date_str, EXESS_SHORT, EXESS_UNSUPPORTED);

  check_failure(EXESS_DURATION, duration_str, EXESS_BYTE, EXESS_UNSUPPORTED);
  check_failure(EXESS_DATETIME, datetime_str, EXESS_BYTE, EXESS_UNSUPPORTED);
  check_failure(EXESS_TIME, time_str, EXESS_BYTE, EXESS_UNSUPPORTED);
  check_failure(EXESS_DATE, date_str, EXESS_BYTE, EXESS_UNSUPPORTED);

  check_failure(EXESS_DURATION, duration_str, EXESS_ULONG, EXESS_UNSUPPORTED);
  check_failure(EXESS_DATETIME, datetime_str, EXESS_ULONG, EXESS_UNSUPPORTED);
  check_failure(EXESS_TIME, time_str, EXESS_ULONG, EXESS_UNSUPPORTED);
  check_failure(EXESS_DATE, date_str, EXESS_ULONG, EXESS_UNSUPPORTED);

  check_failure(EXESS_DURATION, duration_str, EXESS_UINT, EXESS_UNSUPPORTED);
  check_failure(EXESS_DATETIME, datetime_str, EXESS_UINT, EXESS_UNSUPPORTED);
  check_failure(EXESS_TIME, time_str, EXESS_UINT, EXESS_UNSUPPORTED);
  check_failure(EXESS_DATE, date_str, EXESS_UINT, EXESS_UNSUPPORTED);

  check_failure(EXESS_DURATION, duration_str, EXESS_USHORT, EXESS_UNSUPPORTED);
  check_failure(EXESS_DATETIME, datetime_str, EXESS_USHORT, EXESS_UNSUPPORTED);
  check_failure(EXESS_TIME, time_str, EXESS_USHORT, EXESS_UNSUPPORTED);
  check_failure(EXESS_DATE, date_str, EXESS_USHORT, EXESS_UNSUPPORTED);

  check_failure(EXESS_DURATION, duration_str, EXESS_UBYTE, EXESS_UNSUPPORTED);
  check_failure(EXESS_DATETIME, datetime_str, EXESS_UBYTE, EXESS_UNSUPPORTED);
  check_failure(EXESS_TIME, time_str, EXESS_UBYTE, EXESS_UNSUPPORTED);
  check_failure(EXESS_DATE, date_str, EXESS_UBYTE, EXESS_UNSUPPORTED);
}

static void
test_binary(void)
{
  check_from_to(EXESS_HEX, "666F6F", EXESS_BASE64, "Zm9v");
  check_from_to(EXESS_BASE64, "Zm9v", EXESS_HEX, "666F6F");

  check_failure(EXESS_LONG, "-2147483649", EXESS_HEX, EXESS_UNSUPPORTED);
  check_failure(EXESS_LONG, "-2147483649", EXESS_BASE64, EXESS_UNSUPPORTED);
}

int
main(void)
{
  test_unknown();
  test_boolean();
  test_decimal();
  test_long();
  test_ulong();
  test_large_integers();
  test_lossy();
  test_date_time();
  test_number_to_time();
  test_time_to_number();
  test_binary();

  return 0;
}
