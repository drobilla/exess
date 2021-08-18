// Copyright 2019-2021 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#ifndef EXESS_IEEE_FLOAT_H
#define EXESS_IEEE_FLOAT_H

#include <float.h>
#include <stdint.h>
#include <string.h>

static const unsigned dbl_physical_mant_dig = DBL_MANT_DIG - 1u;
static const uint64_t dbl_mant_mask         = 0x000FFFFFFFFFFFFFull;
static const uint64_t dbl_expt_mask         = 0x7FF0000000000000ul;
static const uint64_t dbl_hidden_bit        = 0x0010000000000000ul;
static const int      dbl_expt_bias         = 0x3FF + DBL_MANT_DIG - 1;
static const int      dbl_subnormal_expt    = -0x3FF - DBL_MANT_DIG + 2;

/// Return the raw representation of a float
static inline uint32_t
float_to_rep(const float d)
{
  uint32_t rep = 0;
  memcpy(&rep, &d, sizeof(rep));
  return rep;
}

/// Return the raw representation of a double
static inline uint64_t
double_to_rep(const double d)
{
  uint64_t rep = 0;
  memcpy(&rep, &d, sizeof(rep));
  return rep;
}

#endif // EXESS_IEEE_FLOAT_H
