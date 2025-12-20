// Copyright 2011-2025 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#undef NDEBUG

#include "write_test_utils.h"

#include <exess/exess.h>

#include <assert.h>
#include <stdint.h>
#include <string.h>

#define VALUE_SIZE EXESS_MAX_FIXED_SIZE
#define STRING_SIZE (EXESS_MAX_DECIMAL_LENGTH + 1U)

static const char* const min_long_str  = "-9223372036854775808";
static const char* const max_long_str  = "9223372036854775807";
static const char* const max_ulong_str = "18446744073709551615";

typedef struct {
  EXESS_ALIGN uint8_t value[VALUE_SIZE];
  EXESS_ALIGN uint8_t coerced[VALUE_SIZE];
  ExessVariableResult r_value;
  ExessResult         r_coerced;
} CheckState;

static void
check_conversions(CheckState* const    state,
                  const ExessDatatype  from_datatype,
                  const char* const    from_string,
                  const ExessCoercions coercions,
                  const ExessDatatype  to_datatype,
                  const char* const    to_string)
{
  char coerced_str[STRING_SIZE] = {42};
  init_out_buf(sizeof(coerced_str), coerced_str);

  // Read original value
  state->r_value = exess_read_value(
    from_datatype, from_string, sizeof(state->value), state->value);

  assert(!state->r_value.status);

  // Coerce to target datatype
  state->r_coerced = exess_coerce_value(coercions,
                                        from_datatype,
                                        state->r_value.write_count,
                                        state->value,
                                        to_datatype,
                                        sizeof(state->coerced),
                                        state->coerced);

  assert(!state->r_coerced.status);
  assert(state->r_coerced.count >= exess_value_sizes[to_datatype]);

  // Write coerced value and check string against expectation
  const ExessResult coerced_str_r = exess_write_value(to_datatype,
                                                      state->r_coerced.count,
                                                      state->coerced,
                                                      sizeof(coerced_str),
                                                      coerced_str);

  assert(!coerced_str_r.status);
  assert(!strcmp(coerced_str, to_string));
}

// Check a perfect (lossless/bidirectional) coercion
static void
check_perf(const ExessDatatype from_datatype,
           const char* const   from_string,
           const ExessDatatype to_datatype,
           const char* const   to_string)
{
  CheckState state = {{0}, {0}, {EXESS_SUCCESS, 0, 0}, {EXESS_SUCCESS, 0}};

  check_conversions(
    &state, from_datatype, from_string, EXESS_LOSSLESS, to_datatype, to_string);

  EXESS_ALIGN uint8_t tripped[VALUE_SIZE]      = {0};
  char                tripped_str[STRING_SIZE] = {42};
  init_out_buf(sizeof(tripped_str), tripped_str);

  // Coerce the value back to the original type
  const ExessResult tripped_r = exess_coerce_value(EXESS_LOSSLESS,
                                                   to_datatype,
                                                   state.r_coerced.count,
                                                   state.coerced,
                                                   from_datatype,
                                                   sizeof(tripped),
                                                   tripped);

  assert(!tripped_r.status);
  assert(tripped_r.count >= exess_value_sizes[from_datatype]);

  // Write round-tripped value and check string against the original
  const ExessResult tr = exess_write_value(
    from_datatype, tripped_r.count, tripped, sizeof(tripped_str), tripped_str);

  assert(!tr.status);
  assert(!strcmp(tripped_str, from_string));
}

// Check that a failed lossless coercion returns the expected error
static void
check_fail(const ExessDatatype from_datatype,
           const char* const   from_string,
           const ExessDatatype to_datatype,
           const ExessStatus   expected_status)
{
  EXESS_ALIGN uint8_t value[VALUE_SIZE]   = {0};
  EXESS_ALIGN uint8_t coerced[VALUE_SIZE] = {0};

  // Read original value
  const ExessVariableResult vr =
    exess_read_value(from_datatype, from_string, sizeof(value), value);

  assert(!vr.status);

  // Try to coerce to target datatype
  const ExessResult coerced_r = exess_coerce_value(EXESS_LOSSLESS,
                                                   from_datatype,
                                                   vr.write_count,
                                                   value,
                                                   to_datatype,
                                                   sizeof(coerced),
                                                   coerced);

  assert(coerced_r.status == expected_status);
}

// Check a lossy coercion in one direction only
static void
check_loss(const ExessDatatype  from_datatype,
           const char* const    from_string,
           const ExessDatatype  to_datatype,
           const char* const    to_string,
           const ExessStatus    loss_status,
           const ExessCoercions coercions)
{
  CheckState state = {{0}, {0}, {EXESS_SUCCESS, 0, 0}, {EXESS_SUCCESS, 0}};

  check_conversions(
    &state, from_datatype, from_string, coercions, to_datatype, to_string);

  assert(exess_coerce_value(EXESS_LOSSLESS,
                            from_datatype,
                            state.r_value.write_count,
                            state.value,
                            to_datatype,
                            state.r_coerced.count,
                            state.coerced)
           .status == loss_status);
}

// Check a widening coercions that's only lossless in one direction
static void
check_wide(const ExessDatatype from_datatype,
           const char* const   from_string,
           const ExessDatatype to_datatype,
           const char* const   to_string,
           const ExessStatus   reverse_status)
{
  CheckState state = {{0}, {0}, {EXESS_SUCCESS, 0, 0}, {EXESS_SUCCESS, 0}};

  check_conversions(
    &state, from_datatype, from_string, EXESS_LOSSLESS, to_datatype, to_string);

  // Ensure that converting coerced value back isn't lossless
  assert(exess_coerce_value(EXESS_LOSSLESS,
                            to_datatype,
                            state.r_coerced.count,
                            state.coerced,
                            from_datatype,
                            state.r_value.write_count,
                            state.value)
           .status == reverse_status);
}

// Check a truncating coercion
static void
check_trunc(const ExessDatatype from_datatype,
            const char* const   from_string,
            const ExessDatatype to_datatype,
            const char* const   to_string)
{
  check_loss(from_datatype,
             from_string,
             to_datatype,
             to_string,
             EXESS_WOULD_TRUNCATE,
             EXESS_TRUNCATE);
}

// Check a rounding coercion
static void
check_round(const ExessDatatype from_datatype,
            const char* const   from_string,
            const ExessDatatype to_datatype,
            const char* const   to_string)
{
  check_loss(from_datatype,
             from_string,
             to_datatype,
             to_string,
             EXESS_WOULD_ROUND,
             EXESS_ROUND);
}

static void
test_overflow(void)
{
  const uint32_t v = 4294967295U;
  uint16_t       o = 42U;

  ExessResult r = exess_coerce_value(
    EXESS_LOSSLESS, EXESS_SHORT, 1U, &v, EXESS_LONG, sizeof(o), &o);

  assert(r.status == EXESS_BAD_VALUE);
  assert(o == 42U);

  r = exess_coerce_value(
    EXESS_LOSSLESS, EXESS_SHORT, sizeof(v), &v, EXESS_LONG, sizeof(o), &o);

  assert(r.status == EXESS_NO_SPACE);
  assert(o == 42U);

  r = exess_coerce_value(
    EXESS_LOSSLESS, EXESS_HEX, sizeof(v), &v, EXESS_BASE64, sizeof(o), &o);

  assert(r.status == EXESS_NO_SPACE);
  assert(o == 42U);
}

static void
test_unknown(void)
{
  const int64_t  long_value  = 1;
  const uint64_t ulong_value = 1U;

  uint8_t in[32]  = {0U};
  uint8_t out[32] = {0U};

  assert(exess_coerce_value(EXESS_LOSSLESS,
                            EXESS_NOTHING,
                            sizeof(in),
                            in,
                            EXESS_LONG,
                            sizeof(out),
                            out)
           .status);

  assert(exess_coerce_value(EXESS_LOSSLESS,
                            EXESS_NOTHING,
                            sizeof(in),
                            in,
                            EXESS_ULONG,
                            sizeof(out),
                            out)
           .status);

  assert(exess_coerce_value(EXESS_LOSSLESS,
                            EXESS_LONG,
                            sizeof(long_value),
                            &long_value,
                            EXESS_NOTHING,
                            sizeof(out),
                            out)
           .status);

  assert(exess_coerce_value(EXESS_LOSSLESS,
                            EXESS_ULONG,
                            sizeof(ulong_value),
                            &ulong_value,
                            EXESS_NOTHING,
                            sizeof(out),
                            out)
           .status);
}

static void
test_floating(void)
{
  check_wide(
    EXESS_FLOAT, "1.2E3", EXESS_DOUBLE, "1.2E3", EXESS_WOULD_REDUCE_PRECISION);

  check_loss(EXESS_DOUBLE,
             "1.0000000000001E0",
             EXESS_FLOAT,
             "1.0E0",
             EXESS_WOULD_REDUCE_PRECISION,
             EXESS_REDUCE_PRECISION);

  check_loss(EXESS_DOUBLE,
             "1.5E0",
             EXESS_FLOAT,
             "1.5E0",
             EXESS_WOULD_REDUCE_PRECISION,
             EXESS_REDUCE_PRECISION);
}

static void
test_decimal(void)
{
  check_perf(EXESS_DECIMAL, "1.2", EXESS_DECIMAL, "1.2");
  check_perf(EXESS_DOUBLE, "1.2E4", EXESS_DECIMAL, "12000.0");

  check_wide(EXESS_FLOAT,
             "1.2E3",
             EXESS_DECIMAL,
             "1200.0",
             EXESS_WOULD_REDUCE_PRECISION);

  check_loss(EXESS_DECIMAL,
             "1.000000001",
             EXESS_FLOAT,
             "1.0E0",
             EXESS_WOULD_REDUCE_PRECISION,
             EXESS_REDUCE_PRECISION);

  check_perf(
    EXESS_DECIMAL, "-9007199254740991.0", EXESS_LONG, "-9007199254740991");

  check_fail(
    EXESS_DECIMAL, "-9007199254740992.0", EXESS_LONG, EXESS_OUT_OF_RANGE);

  check_fail(
    EXESS_DECIMAL, "9007199254740992.0", EXESS_ULONG, EXESS_OUT_OF_RANGE);

  check_fail(
    EXESS_ULONG, "18446744073709551615", EXESS_DECIMAL, EXESS_OUT_OF_RANGE);

  check_fail(
    EXESS_LONG, "-9007199254740992", EXESS_DECIMAL, EXESS_OUT_OF_RANGE);
}

static void
test_boolean(void)
{
  // Exactly from 0 or 1, lossy from 0 or non-zero
  check_perf(EXESS_BOOLEAN, "false", EXESS_FLOAT, "0.0E0");
  check_perf(EXESS_BOOLEAN, "true", EXESS_FLOAT, "1.0E0");
  check_perf(EXESS_BOOLEAN, "false", EXESS_DOUBLE, "0.0E0");
  check_perf(EXESS_BOOLEAN, "true", EXESS_DOUBLE, "1.0E0");
  check_perf(EXESS_BOOLEAN, "false", EXESS_LONG, "0");
  check_perf(EXESS_BOOLEAN, "true", EXESS_LONG, "1");
  check_fail(EXESS_LONG, "-1", EXESS_BOOLEAN, EXESS_WOULD_TRUNCATE);
  check_fail(EXESS_LONG, "2", EXESS_BOOLEAN, EXESS_WOULD_TRUNCATE);
  check_trunc(EXESS_LONG, "42", EXESS_BOOLEAN, "true");
  check_trunc(EXESS_LONG, "-1", EXESS_BOOLEAN, "true");
  check_perf(EXESS_BOOLEAN, "false", EXESS_ULONG, "0");
  check_perf(EXESS_BOOLEAN, "true", EXESS_ULONG, "1");
  check_fail(EXESS_LONG, "2", EXESS_BOOLEAN, EXESS_WOULD_TRUNCATE);
  check_trunc(EXESS_ULONG, "42", EXESS_BOOLEAN, "true");

  // Not convertible to any time types
  check_fail(EXESS_BOOLEAN, "true", EXESS_DURATION, EXESS_UNSUPPORTED);
  check_fail(EXESS_BOOLEAN, "true", EXESS_DATE_TIME, EXESS_UNSUPPORTED);
  check_fail(EXESS_BOOLEAN, "true", EXESS_TIME, EXESS_UNSUPPORTED);
  check_fail(EXESS_BOOLEAN, "true", EXESS_DATE, EXESS_UNSUPPORTED);
}

static void
test_round(void)
{
  static const ExessDatatype float_types[] = {
    EXESS_DOUBLE, EXESS_FLOAT, EXESS_NOTHING};
  static const ExessDatatype signed_types[] = {
    EXESS_LONG, EXESS_INT, EXESS_SHORT, EXESS_BYTE, EXESS_NOTHING};
  static const ExessDatatype unsigned_types[] = {
    EXESS_ULONG, EXESS_UINT, EXESS_USHORT, EXESS_UBYTE, EXESS_NOTHING};

  for (const ExessDatatype* f = float_types; *f; ++f) {
    for (const ExessDatatype* t = signed_types; *t; ++t) {
      check_round(*f, "-3.5E0", *t, "-4");
      check_round(*f, "-2.5E0", *t, "-2");
      check_round(*f, "-1.5E0", *t, "-2");
      check_round(*f, "-1.4E0", *t, "-1");
      check_round(*f, "-0.5E0", *t, "0");
      check_round(*f, "0.5E0", *t, "0");
      check_round(*f, "1.4E0", *t, "1");
      check_round(*f, "1.5E0", *t, "2");
      check_round(*f, "2.5E0", *t, "2");
      check_round(*f, "3.5E0", *t, "4");
    }

    for (const ExessDatatype* t = unsigned_types; *t; ++t) {
      check_round(*f, "0.5E0", *t, "0");
      check_round(*f, "1.5E0", *t, "2");
      check_round(*f, "2.5E0", *t, "2");
      check_round(*f, "3.5E0", *t, "4");
    }
  }
}

static void
test_long(void)
{
  // Truncating boolean conversion
  check_trunc(EXESS_LONG, "42", EXESS_BOOLEAN, "true");
  check_trunc(EXESS_LONG, "-1", EXESS_BOOLEAN, "true");

  // All smaller integer types
  check_perf(EXESS_LONG, "-2147483648", EXESS_INT, "-2147483648");
  check_fail(EXESS_LONG, "-2147483649", EXESS_INT, EXESS_OUT_OF_RANGE);
  check_perf(EXESS_LONG, "-32768", EXESS_SHORT, "-32768");
  check_fail(EXESS_LONG, "-32769", EXESS_SHORT, EXESS_OUT_OF_RANGE);
  check_perf(EXESS_LONG, "-128", EXESS_BYTE, "-128");
  check_fail(EXESS_LONG, "-129", EXESS_BYTE, EXESS_OUT_OF_RANGE);

  // Positive values to/from all unsigned types
  check_perf(EXESS_LONG, max_long_str, EXESS_ULONG, max_long_str);
  check_fail(EXESS_LONG, "-1", EXESS_ULONG, EXESS_OUT_OF_RANGE);
  check_perf(EXESS_LONG, "4294967295", EXESS_UINT, "4294967295");
  check_fail(EXESS_LONG, "-1", EXESS_UINT, EXESS_OUT_OF_RANGE);
  check_perf(EXESS_LONG, "65535", EXESS_USHORT, "65535");
  check_fail(EXESS_LONG, "-1", EXESS_USHORT, EXESS_OUT_OF_RANGE);
  check_perf(EXESS_LONG, "255", EXESS_UBYTE, "255");
  check_fail(EXESS_LONG, "-1", EXESS_UBYTE, EXESS_OUT_OF_RANGE);

  // Any value to/from integer
  check_perf(EXESS_LONG, min_long_str, EXESS_INTEGER, min_long_str);
  check_perf(EXESS_LONG, max_long_str, EXESS_INTEGER, max_long_str);

  // Non-positive values to/from nonPositiveInteger
  check_perf(
    EXESS_LONG, min_long_str, EXESS_NON_POSITIVE_INTEGER, min_long_str);
  check_perf(EXESS_LONG, "0", EXESS_NON_POSITIVE_INTEGER, "0");
  check_fail(EXESS_LONG, "1", EXESS_NON_POSITIVE_INTEGER, EXESS_OUT_OF_RANGE);

  // Negative values to/from negativeInteger
  check_perf(EXESS_LONG, min_long_str, EXESS_NEGATIVE_INTEGER, min_long_str);
  check_perf(EXESS_LONG, "-1", EXESS_NEGATIVE_INTEGER, "-1");
  check_fail(EXESS_LONG, "0", EXESS_NEGATIVE_INTEGER, EXESS_OUT_OF_RANGE);

  // Non-negative values to/from nonNegativeInteger
  check_fail(EXESS_LONG, "-1", EXESS_NON_NEGATIVE_INTEGER, EXESS_OUT_OF_RANGE);
  check_perf(EXESS_LONG, "0", EXESS_NON_NEGATIVE_INTEGER, "0");
  check_perf(
    EXESS_LONG, max_long_str, EXESS_NON_NEGATIVE_INTEGER, max_long_str);

  // Positive values to/from positiveInteger
  check_fail(EXESS_LONG, "-1", EXESS_POSITIVE_INTEGER, EXESS_OUT_OF_RANGE);
  check_fail(EXESS_LONG, "0", EXESS_POSITIVE_INTEGER, EXESS_OUT_OF_RANGE);
  check_perf(EXESS_LONG, max_long_str, EXESS_POSITIVE_INTEGER, max_long_str);
  check_fail(EXESS_POSITIVE_INTEGER,
             "9223372036854775808",
             EXESS_LONG,
             EXESS_OUT_OF_RANGE);

  // Float
  check_fail(EXESS_FLOAT, "1.5", EXESS_LONG, EXESS_WOULD_ROUND);
  check_fail(EXESS_FLOAT, "1E38", EXESS_LONG, EXESS_OUT_OF_RANGE);
  check_perf(EXESS_LONG, "-16777215", EXESS_FLOAT, "-1.6777215E7");
  check_fail(EXESS_LONG, "-16777216", EXESS_FLOAT, EXESS_OUT_OF_RANGE);
  check_fail(EXESS_FLOAT, "-16777216", EXESS_LONG, EXESS_OUT_OF_RANGE);
  check_perf(EXESS_LONG, "16777215", EXESS_FLOAT, "1.6777215E7");
  check_fail(EXESS_LONG, "16777216", EXESS_FLOAT, EXESS_OUT_OF_RANGE);
  check_fail(EXESS_FLOAT, "16777216", EXESS_LONG, EXESS_OUT_OF_RANGE);
  check_perf(EXESS_FLOAT, "1.0E0", EXESS_LONG, "1");
  check_round(EXESS_FLOAT, "1.5E0", EXESS_LONG, "2");
  check_round(EXESS_FLOAT, "2.5E0", EXESS_LONG, "2");
  check_round(EXESS_FLOAT, "3.5E0", EXESS_LONG, "4");

  // Double
  check_fail(EXESS_DOUBLE, "1.5", EXESS_LONG, EXESS_WOULD_ROUND);
  check_fail(EXESS_DOUBLE, "1E38", EXESS_LONG, EXESS_OUT_OF_RANGE);
  check_perf(
    EXESS_LONG, "-9007199254740991", EXESS_DOUBLE, "-9.007199254740991E15");
  check_fail(EXESS_LONG, "-9007199254740992", EXESS_DOUBLE, EXESS_OUT_OF_RANGE);
  check_fail(EXESS_DOUBLE, "-9007199254740992", EXESS_LONG, EXESS_OUT_OF_RANGE);
  check_perf(
    EXESS_LONG, "9007199254740991", EXESS_DOUBLE, "9.007199254740991E15");
  check_fail(EXESS_LONG, "9007199254740992", EXESS_DOUBLE, EXESS_OUT_OF_RANGE);
  check_fail(EXESS_DOUBLE, "9007199254740992", EXESS_LONG, EXESS_OUT_OF_RANGE);
  check_perf(EXESS_DOUBLE, "1.0E0", EXESS_LONG, "1");
  check_round(EXESS_DOUBLE, "1.5E0", EXESS_LONG, "2");
  check_round(EXESS_DOUBLE, "2.5E0", EXESS_LONG, "2");
  check_round(EXESS_DOUBLE, "3.5E0", EXESS_LONG, "4");
}

static void
test_ulong(void)
{
  // Truncating boolean conversion
  check_trunc(EXESS_ULONG, "42", EXESS_BOOLEAN, "true");

  // All integer types
  check_perf(EXESS_ULONG, max_long_str, EXESS_LONG, max_long_str);
  check_fail(EXESS_ULONG, max_ulong_str, EXESS_LONG, EXESS_OUT_OF_RANGE);
  check_perf(EXESS_ULONG, "2147483647", EXESS_INT, "2147483647");
  check_fail(EXESS_ULONG, "2147483648", EXESS_INT, EXESS_OUT_OF_RANGE);
  check_fail(EXESS_INT, "-1", EXESS_ULONG, EXESS_OUT_OF_RANGE);
  check_perf(EXESS_ULONG, "32767", EXESS_SHORT, "32767");
  check_fail(EXESS_ULONG, "32768", EXESS_SHORT, EXESS_OUT_OF_RANGE);
  check_fail(EXESS_SHORT, "-1", EXESS_ULONG, EXESS_OUT_OF_RANGE);
  check_perf(EXESS_ULONG, "127", EXESS_BYTE, "127");
  check_fail(EXESS_ULONG, "128", EXESS_BYTE, EXESS_OUT_OF_RANGE);
  check_fail(EXESS_BYTE, "-1", EXESS_ULONG, EXESS_OUT_OF_RANGE);

  // All unsigned types
  check_perf(EXESS_ULONG, "4294967295", EXESS_UINT, "4294967295");
  check_fail(EXESS_ULONG, "4294967296", EXESS_UINT, EXESS_OUT_OF_RANGE);
  check_perf(EXESS_ULONG, "65535", EXESS_USHORT, "65535");
  check_fail(EXESS_ULONG, "65536", EXESS_USHORT, EXESS_OUT_OF_RANGE);
  check_perf(EXESS_ULONG, "255", EXESS_UBYTE, "255");
  check_fail(EXESS_ULONG, "256", EXESS_UBYTE, EXESS_OUT_OF_RANGE);

  // Limits
  check_perf(EXESS_ULONG, "0", EXESS_INTEGER, "0");
  check_fail(
    EXESS_ULONG, "9223372036854775808", EXESS_INTEGER, EXESS_OUT_OF_RANGE);

  // Only zero to/from nonPositiveInteger
  check_perf(EXESS_ULONG, "0", EXESS_NON_POSITIVE_INTEGER, "0");
  check_fail(EXESS_ULONG, "1", EXESS_NON_POSITIVE_INTEGER, EXESS_OUT_OF_RANGE);

  // Not convertible to/from negativeInteger
  check_fail(EXESS_ULONG, "0", EXESS_NEGATIVE_INTEGER, EXESS_OUT_OF_RANGE);
  check_fail(EXESS_ULONG, "1", EXESS_NEGATIVE_INTEGER, EXESS_OUT_OF_RANGE);

  // Any value to/from nonNegativeInteger
  check_perf(EXESS_ULONG, "0", EXESS_NON_NEGATIVE_INTEGER, "0");
  check_perf(
    EXESS_ULONG, max_ulong_str, EXESS_NON_NEGATIVE_INTEGER, max_ulong_str);

  // Positive values to/from positiveInteger
  check_fail(EXESS_ULONG, "0", EXESS_POSITIVE_INTEGER, EXESS_OUT_OF_RANGE);
  check_perf(EXESS_ULONG, "1", EXESS_POSITIVE_INTEGER, "1");

  // Float
  check_fail(EXESS_FLOAT, "-1", EXESS_ULONG, EXESS_OUT_OF_RANGE);
  check_fail(EXESS_FLOAT, "1.5", EXESS_ULONG, EXESS_WOULD_ROUND);
  check_perf(EXESS_ULONG, "0", EXESS_FLOAT, "0.0E0");
  check_perf(EXESS_ULONG, "16777215", EXESS_FLOAT, "1.6777215E7");
  check_fail(EXESS_ULONG, "16777216", EXESS_FLOAT, EXESS_OUT_OF_RANGE);
  check_fail(EXESS_FLOAT, "16777216", EXESS_ULONG, EXESS_OUT_OF_RANGE);
  check_perf(EXESS_FLOAT, "1.0E0", EXESS_ULONG, "1");
  check_round(EXESS_FLOAT, "1.5E0", EXESS_ULONG, "2");
  check_round(EXESS_FLOAT, "2.5E0", EXESS_ULONG, "2");
  check_round(EXESS_FLOAT, "3.5E0", EXESS_ULONG, "4");

  // Double
  check_fail(EXESS_DOUBLE, "-1", EXESS_ULONG, EXESS_OUT_OF_RANGE);
  check_fail(EXESS_DOUBLE, "1.5", EXESS_ULONG, EXESS_WOULD_ROUND);
  check_perf(EXESS_ULONG, "0", EXESS_DOUBLE, "0.0E0");
  check_perf(
    EXESS_ULONG, "9007199254740991", EXESS_DOUBLE, "9.007199254740991E15");
  check_fail(EXESS_ULONG, "9007199254740992", EXESS_DOUBLE, EXESS_OUT_OF_RANGE);
  check_fail(EXESS_DOUBLE, "9007199254740992", EXESS_ULONG, EXESS_OUT_OF_RANGE);
  check_perf(EXESS_DOUBLE, "1.0E0", EXESS_ULONG, "1");
  check_round(EXESS_DOUBLE, "1.5E0", EXESS_ULONG, "2");
  check_round(EXESS_DOUBLE, "2.5E0", EXESS_ULONG, "2");
  check_round(EXESS_DOUBLE, "3.5E0", EXESS_ULONG, "4");
}

static void
test_large_integers(void)
{
  check_fail(EXESS_TIME, "00:00:00", EXESS_INTEGER, EXESS_UNSUPPORTED);
  check_fail(
    EXESS_TIME, "00:00:00", EXESS_NON_POSITIVE_INTEGER, EXESS_UNSUPPORTED);
  check_fail(EXESS_TIME, "00:00:00", EXESS_NEGATIVE_INTEGER, EXESS_UNSUPPORTED);
  check_fail(
    EXESS_TIME, "00:00:00", EXESS_NON_NEGATIVE_INTEGER, EXESS_UNSUPPORTED);
  check_fail(EXESS_TIME, "00:00:00", EXESS_POSITIVE_INTEGER, EXESS_UNSUPPORTED);
}

static void
test_date_time(void)
{
  check_fail(
    EXESS_DATE_TIME, "2001-02-03T04:05:06", EXESS_TIME, EXESS_WOULD_TRUNCATE);

  check_trunc(EXESS_DATE_TIME, "2001-02-03T04:05:06", EXESS_TIME, "04:05:06");

  check_trunc(EXESS_DATE_TIME, "2001-02-03T04:05:06Z", EXESS_TIME, "04:05:06Z");

  check_trunc(
    EXESS_DATE_TIME, "2001-02-03T04:05:06+07:30", EXESS_TIME, "04:05:06+07:30");

  check_trunc(EXESS_DATE_TIME, "2001-02-03T04:05:06", EXESS_DATE, "2001-02-03");

  check_trunc(
    EXESS_DATE_TIME, "2001-02-03T04:05:06Z", EXESS_DATE, "2001-02-03Z");

  check_trunc(EXESS_DATE_TIME,
              "2001-02-03T04:05:06+07:30",
              EXESS_DATE,
              "2001-02-03+07:30");
}

static void
test_number_to_time(void)
{
  check_fail(EXESS_BOOLEAN, "true", EXESS_DURATION, EXESS_UNSUPPORTED);
  check_fail(EXESS_BOOLEAN, "true", EXESS_DATE_TIME, EXESS_UNSUPPORTED);
  check_fail(EXESS_BOOLEAN, "true", EXESS_TIME, EXESS_UNSUPPORTED);
  check_fail(EXESS_BOOLEAN, "true", EXESS_DATE, EXESS_UNSUPPORTED);

  check_fail(EXESS_LONG, "1", EXESS_DURATION, EXESS_UNSUPPORTED);
  check_fail(EXESS_LONG, "1", EXESS_DATE_TIME, EXESS_UNSUPPORTED);
  check_fail(EXESS_LONG, "1", EXESS_TIME, EXESS_UNSUPPORTED);
  check_fail(EXESS_LONG, "1", EXESS_DATE, EXESS_UNSUPPORTED);

  check_fail(EXESS_INT, "1", EXESS_DURATION, EXESS_UNSUPPORTED);
  check_fail(EXESS_INT, "1", EXESS_DATE_TIME, EXESS_UNSUPPORTED);
  check_fail(EXESS_INT, "1", EXESS_TIME, EXESS_UNSUPPORTED);
  check_fail(EXESS_INT, "1", EXESS_DATE, EXESS_UNSUPPORTED);

  check_fail(EXESS_SHORT, "1", EXESS_DURATION, EXESS_UNSUPPORTED);
  check_fail(EXESS_SHORT, "1", EXESS_DATE_TIME, EXESS_UNSUPPORTED);
  check_fail(EXESS_SHORT, "1", EXESS_TIME, EXESS_UNSUPPORTED);
  check_fail(EXESS_SHORT, "1", EXESS_DATE, EXESS_UNSUPPORTED);

  check_fail(EXESS_BYTE, "1", EXESS_DURATION, EXESS_UNSUPPORTED);
  check_fail(EXESS_BYTE, "1", EXESS_DATE_TIME, EXESS_UNSUPPORTED);
  check_fail(EXESS_BYTE, "1", EXESS_TIME, EXESS_UNSUPPORTED);
  check_fail(EXESS_BYTE, "1", EXESS_DATE, EXESS_UNSUPPORTED);

  check_fail(EXESS_ULONG, "1", EXESS_DURATION, EXESS_UNSUPPORTED);
  check_fail(EXESS_ULONG, "1", EXESS_DATE_TIME, EXESS_UNSUPPORTED);
  check_fail(EXESS_ULONG, "1", EXESS_TIME, EXESS_UNSUPPORTED);
  check_fail(EXESS_ULONG, "1", EXESS_DATE, EXESS_UNSUPPORTED);

  check_fail(EXESS_UINT, "1", EXESS_DURATION, EXESS_UNSUPPORTED);
  check_fail(EXESS_UINT, "1", EXESS_DATE_TIME, EXESS_UNSUPPORTED);
  check_fail(EXESS_UINT, "1", EXESS_TIME, EXESS_UNSUPPORTED);
  check_fail(EXESS_UINT, "1", EXESS_DATE, EXESS_UNSUPPORTED);

  check_fail(EXESS_USHORT, "1", EXESS_DURATION, EXESS_UNSUPPORTED);
  check_fail(EXESS_USHORT, "1", EXESS_DATE_TIME, EXESS_UNSUPPORTED);
  check_fail(EXESS_USHORT, "1", EXESS_TIME, EXESS_UNSUPPORTED);
  check_fail(EXESS_USHORT, "1", EXESS_DATE, EXESS_UNSUPPORTED);

  check_fail(EXESS_UBYTE, "1", EXESS_DURATION, EXESS_UNSUPPORTED);
  check_fail(EXESS_UBYTE, "1", EXESS_DATE_TIME, EXESS_UNSUPPORTED);
  check_fail(EXESS_UBYTE, "1", EXESS_TIME, EXESS_UNSUPPORTED);
  check_fail(EXESS_UBYTE, "1", EXESS_DATE, EXESS_UNSUPPORTED);
}

static void
test_time_to_number(void)
{
  static const char* const duration_str = "P1Y";
  static const char* const datetime_str = "2001-02-03T04:05:06";
  static const char* const time_str     = "04:05:06";
  static const char* const date_str     = "2001-02-03";

  check_fail(EXESS_DURATION, duration_str, EXESS_BOOLEAN, EXESS_UNSUPPORTED);
  check_fail(EXESS_DATE_TIME, datetime_str, EXESS_BOOLEAN, EXESS_UNSUPPORTED);
  check_fail(EXESS_TIME, time_str, EXESS_BOOLEAN, EXESS_UNSUPPORTED);
  check_fail(EXESS_DATE, date_str, EXESS_BOOLEAN, EXESS_UNSUPPORTED);

  check_fail(EXESS_DURATION, duration_str, EXESS_INT, EXESS_UNSUPPORTED);
  check_fail(EXESS_DATE_TIME, datetime_str, EXESS_INT, EXESS_UNSUPPORTED);
  check_fail(EXESS_TIME, time_str, EXESS_INT, EXESS_UNSUPPORTED);
  check_fail(EXESS_DATE, date_str, EXESS_INT, EXESS_UNSUPPORTED);

  check_fail(EXESS_DURATION, duration_str, EXESS_SHORT, EXESS_UNSUPPORTED);
  check_fail(EXESS_DATE_TIME, datetime_str, EXESS_SHORT, EXESS_UNSUPPORTED);
  check_fail(EXESS_TIME, time_str, EXESS_SHORT, EXESS_UNSUPPORTED);
  check_fail(EXESS_DATE, date_str, EXESS_SHORT, EXESS_UNSUPPORTED);

  check_fail(EXESS_DURATION, duration_str, EXESS_BYTE, EXESS_UNSUPPORTED);
  check_fail(EXESS_DATE_TIME, datetime_str, EXESS_BYTE, EXESS_UNSUPPORTED);
  check_fail(EXESS_TIME, time_str, EXESS_BYTE, EXESS_UNSUPPORTED);
  check_fail(EXESS_DATE, date_str, EXESS_BYTE, EXESS_UNSUPPORTED);

  check_fail(EXESS_DURATION, duration_str, EXESS_ULONG, EXESS_UNSUPPORTED);
  check_fail(EXESS_DATE_TIME, datetime_str, EXESS_ULONG, EXESS_UNSUPPORTED);
  check_fail(EXESS_TIME, time_str, EXESS_ULONG, EXESS_UNSUPPORTED);
  check_fail(EXESS_DATE, date_str, EXESS_ULONG, EXESS_UNSUPPORTED);

  check_fail(EXESS_DURATION, duration_str, EXESS_UINT, EXESS_UNSUPPORTED);
  check_fail(EXESS_DATE_TIME, datetime_str, EXESS_UINT, EXESS_UNSUPPORTED);
  check_fail(EXESS_TIME, time_str, EXESS_UINT, EXESS_UNSUPPORTED);
  check_fail(EXESS_DATE, date_str, EXESS_UINT, EXESS_UNSUPPORTED);

  check_fail(EXESS_DURATION, duration_str, EXESS_USHORT, EXESS_UNSUPPORTED);
  check_fail(EXESS_DATE_TIME, datetime_str, EXESS_USHORT, EXESS_UNSUPPORTED);
  check_fail(EXESS_TIME, time_str, EXESS_USHORT, EXESS_UNSUPPORTED);
  check_fail(EXESS_DATE, date_str, EXESS_USHORT, EXESS_UNSUPPORTED);

  check_fail(EXESS_DURATION, duration_str, EXESS_UBYTE, EXESS_UNSUPPORTED);
  check_fail(EXESS_DATE_TIME, datetime_str, EXESS_UBYTE, EXESS_UNSUPPORTED);
  check_fail(EXESS_TIME, time_str, EXESS_UBYTE, EXESS_UNSUPPORTED);
  check_fail(EXESS_DATE, date_str, EXESS_UBYTE, EXESS_UNSUPPORTED);
}

static void
test_binary(void)
{
  check_perf(EXESS_HEX, "666F6F", EXESS_BASE64, "Zm9v");
  check_perf(EXESS_BASE64, "Zm9v", EXESS_HEX, "666F6F");

  check_fail(EXESS_LONG, "-2147483649", EXESS_HEX, EXESS_UNSUPPORTED);
  check_fail(EXESS_LONG, "-2147483649", EXESS_BASE64, EXESS_UNSUPPORTED);
}

int
main(void)
{
  test_overflow();
  test_unknown();
  test_boolean();
  test_floating();
  test_decimal();
  test_round();
  test_long();
  test_ulong();
  test_large_integers();
  test_date_time();
  test_number_to_time();
  test_time_to_number();
  test_binary();

  return 0;
}
