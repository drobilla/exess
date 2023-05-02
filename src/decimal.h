// Copyright 2019-2021 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#ifndef EXESS_SRC_DECIMAL_H
#define EXESS_SRC_DECIMAL_H

// Define C11 numeric constants if the compiler hasn't already
#ifndef FLT_DECIMAL_DIG
#  define FLT_DECIMAL_DIG 9
#endif
#ifndef DBL_DECIMAL_DIG
#  define DBL_DECIMAL_DIG 17
#endif

typedef enum {
  EXESS_NEGATIVE,
  EXESS_NEGATIVE_INFINITY,
  EXESS_NEGATIVE_ZERO,
  EXESS_POSITIVE_ZERO,
  EXESS_POSITIVE,
  EXESS_POSITIVE_INFINITY,
  EXESS_NAN,
} ExessNumberKind;

typedef struct {
  ExessNumberKind kind;                        ///< Kind of number
  int             expt;                        ///< Power of 10 exponent
  unsigned        n_digits;                    ///< Number of significant digits
  char            digits[DBL_DECIMAL_DIG + 2]; ///< Significant digits
} ExessDecimalDouble;

ExessDecimalDouble
exess_measure_float(float f);

ExessDecimalDouble
exess_measure_double(double d);

#endif // EXESS_SRC_DECIMAL_H
