// Copyright 2011-2021 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#undef NDEBUG

#include "exess/exess.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

static const char* const min_long_str  = "-9223372036854775808";
static const char* const max_long_str  = "9223372036854775807";
static const char* const max_ulong_str = "18446744073709551615";

static void
check_conversions(const ExessDatatype  from_datatype,
                  const char* const    from_string,
                  const ExessCoercions coercions,
                  const ExessDatatype  to_datatype,
                  const char* const    to_string,
                  const bool           round_trip)
{
  ExessValue value            = {false};
  ExessValue coerced          = {false};
  char       coerced_str[328] = {42};

  // Read original value
  const ExessVariableResult vr =
    exess_read_value(from_datatype, sizeof(value), &value, from_string);

  assert(!vr.status);

  // Coerce to target datatype
  const ExessResult coerced_r = exess_value_coerce(coercions,
                                                   from_datatype,
                                                   vr.write_count,
                                                   &value,
                                                   to_datatype,
                                                   sizeof(coerced),
                                                   &coerced);

  assert(!coerced_r.status);
  assert(coerced_r.count >= exess_value_size(to_datatype));

  // Write coerced value and check string against expectation
  const ExessResult coerced_str_r = exess_write_value(
    to_datatype, vr.write_count, &coerced, sizeof(coerced_str), coerced_str);

  assert(!coerced_str_r.status);
  assert(!strcmp(coerced_str, to_string));

  if (round_trip) {
    ExessValue tripped          = {false};
    char       tripped_str[328] = {42};

    // Coerce the value back to the original type
    const ExessResult tripped_r = exess_value_coerce(coercions,
                                                     to_datatype,
                                                     coerced_r.count,
                                                     &coerced,
                                                     from_datatype,
                                                     sizeof(tripped),
                                                     &tripped);

    assert(!tripped_r.status);
    assert(tripped_r.count >= exess_value_size(from_datatype));

    // Write round-tripped value and check string against the original
    const ExessResult tr = exess_write_value(from_datatype,
                                             tripped_r.count,
                                             &tripped,
                                             sizeof(tripped_str),
                                             tripped_str);

    assert(!tr.status);
    assert(!strcmp(tripped_str, from_string));
  }
}

static void
check_from_to(const ExessDatatype from_datatype,
              const char* const   from_string,
              const ExessDatatype to_datatype,
              const char* const   to_string)
{
  check_conversions(
    from_datatype, from_string, EXESS_LOSSLESS, to_datatype, to_string, true);
}

static void
check_one_way(const ExessDatatype  from_datatype,
              const char* const    from_string,
              const ExessCoercions coercions,
              const ExessDatatype  to_datatype,
              const char* const    to_string)
{
  check_conversions(
    from_datatype, from_string, coercions, to_datatype, to_string, false);
}

static void
check_failure(const ExessDatatype from_datatype,
              const char* const   from_string,
              const ExessDatatype to_datatype,
              const ExessStatus   expected_status)
{
  ExessValue value   = {false};
  ExessValue coerced = {false};

  // Read original value
  const ExessVariableResult vr =
    exess_read_value(from_datatype, sizeof(value), &value, from_string);

  assert(!vr.status);

  // Try to coerce to target datatype
  const ExessResult coerced_r = exess_value_coerce(EXESS_LOSSLESS,
                                                   from_datatype,
                                                   vr.write_count,
                                                   &value,
                                                   to_datatype,
                                                   sizeof(coerced),
                                                   &coerced);

  assert(coerced_r.status == expected_status);
}

static void
test_overflow(void)
{
  const uint32_t v = 4294967295u;
  uint16_t       o = 42u;

  ExessResult r = exess_value_coerce(
    EXESS_LOSSLESS, EXESS_SHORT, 1u, &v, EXESS_LONG, sizeof(o), &o);

  assert(r.status == EXESS_BAD_VALUE);
  assert(o == 42u);

  r = exess_value_coerce(
    EXESS_LOSSLESS, EXESS_SHORT, sizeof(v), &v, EXESS_LONG, sizeof(o), &o);

  assert(r.status == EXESS_NO_SPACE);
  assert(o == 42u);

  r = exess_value_coerce(
    EXESS_LOSSLESS, EXESS_HEX, sizeof(v), &v, EXESS_BASE64, sizeof(o), &o);

  assert(r.status == EXESS_NO_SPACE);
  assert(o == 42u);
}

static void
test_unknown(void)
{
  const int64_t  long_value  = 1;
  const uint64_t ulong_value = 1u;

  uint8_t in[32]  = {0u};
  uint8_t out[32] = {0u};

  assert(exess_value_coerce(EXESS_LOSSLESS,
                            EXESS_NOTHING,
                            sizeof(in),
                            in,
                            EXESS_LONG,
                            sizeof(out),
                            out)
           .status);

  assert(exess_value_coerce(EXESS_LOSSLESS,
                            EXESS_NOTHING,
                            sizeof(in),
                            in,
                            EXESS_ULONG,
                            sizeof(out),
                            out)
           .status);

  assert(exess_value_coerce(EXESS_LOSSLESS,
                            EXESS_LONG,
                            sizeof(long_value),
                            &long_value,
                            EXESS_NOTHING,
                            sizeof(out),
                            out)
           .status);

  assert(exess_value_coerce(EXESS_LOSSLESS,
                            EXESS_ULONG,
                            sizeof(ulong_value),
                            &ulong_value,
                            EXESS_NOTHING,
                            sizeof(out),
                            out)
           .status);
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
  test_overflow();
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
