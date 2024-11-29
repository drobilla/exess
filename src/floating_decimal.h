// Copyright 2019-2023 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#ifndef EXESS_SRC_FLOATING_DECIMAL_H
#define EXESS_SRC_FLOATING_DECIMAL_H

// Define C11 numeric constants if the compiler hasn't already
#ifndef FLT_DECIMAL_DIG
#  define FLT_DECIMAL_DIG 9
#endif
#ifndef DBL_DECIMAL_DIG
#  define DBL_DECIMAL_DIG 17
#endif

/// A "kind" of floating point number
typedef enum {
  EXESS_NAN,
  EXESS_NEGATIVE_INFINITY,
  EXESS_POSITIVE_INFINITY,
  EXESS_NEGATIVE_ZERO,
  EXESS_POSITIVE_ZERO,
  EXESS_NEGATIVE,
  EXESS_POSITIVE,
} ExessNumberKind;

/// A floating point double stored as a decimal string
typedef struct {
  ExessNumberKind kind;                        ///< Kind of number
  int             expt;                        ///< Power of 10 exponent
  unsigned        n_digits;                    ///< Number of significant digits
  char            digits[DBL_DECIMAL_DIG + 2]; ///< Significant digits
} ExessFloatingDecimal;

ExessFloatingDecimal
measure_float(float f);

ExessFloatingDecimal
measure_double(double d);

#endif // EXESS_SRC_FLOATING_DECIMAL_H
