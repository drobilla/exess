// Copyright 2019-2026 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#ifndef EXESS_SRC_DATE_H
#define EXESS_SRC_DATE_H

#include <exess/exess.h>

/**
   Compare two dates.

   Note that local and zoned dates may not be comparable.  See
   exess_compare_date_time() for details.

   @return Less than, equal to, or greater than zero if `lhs` is less than,
   equal to, or greater than `rhs`, respectively.  Comparable and incomparable
   cases may also be distinguished, see #ExessOrder for details.
*/
EXESS_NONBLOCKING EXESS_CONST_FUNC ExessOrder
exess_compare_date(ExessDate lhs, ExessDate rhs);

#endif // EXESS_SRC_DATE_H
