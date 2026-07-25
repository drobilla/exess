// Copyright 2019-2026 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#ifndef EXESS_SRC_TIME_H
#define EXESS_SRC_TIME_H

#include <exess/exess.h>

/**
   Compare two times.

   Note that local and zoned times may not be comparable.  See
   exess_compare_date_time() for details.

   @return Less than, equal to, or greater than zero if `lhs` is less than,
   equal to, or greater than `rhs`, respectively.  Comparable and incomparable
   cases may also be distinguished, see #ExessOrder for details.
*/
EXESS_NONBLOCKING EXESS_CONST_FUNC ExessOrder
exess_compare_time(ExessTime lhs, ExessTime rhs);

#endif // EXESS_SRC_TIME_H
