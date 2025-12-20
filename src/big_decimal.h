// Copyright 2019-2025 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#ifndef EXESS_SRC_BIG_DECIMAL_H
#define EXESS_SRC_BIG_DECIMAL_H

// Define C11 numeric constants if the compiler hasn't already
#ifndef FLT_DECIMAL_DIG
#  define FLT_DECIMAL_DIG 9
#endif
#ifndef DBL_DECIMAL_DIG
#  define DBL_DECIMAL_DIG 17
#endif

/// A "kind" of decimal floating point number
typedef enum {
  EXESS_NAN,
  EXESS_NEGATIVE_INFINITY,
  EXESS_POSITIVE_INFINITY,
  EXESS_NEGATIVE_ZERO,
  EXESS_POSITIVE_ZERO,
  EXESS_NEGATIVE,
  EXESS_POSITIVE,
} BigDecimalKind;

/**
   An extended range/precision decimal floating point number.

   This is vaguely similar to an IEEE floating point number, except with digits
   stored as ASCII characters.  There's an implicit decimal point before all of
   the digits (that is, an implicit first digit of '0'), and an exponent that
   encodes where the "real" decimal point is.
*/
typedef struct {
  BigDecimalKind kind;                        ///< Kind of number
  int            expt;                        ///< Power of 10 exponent
  unsigned       n_digits;                    ///< Number of significant digits
  char           digits[DBL_DECIMAL_DIG + 2]; ///< Significant digits
} BigDecimal;

#endif // EXESS_SRC_BIG_DECIMAL_H
