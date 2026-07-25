// Copyright 2019-2026 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#ifndef EXESS_SRC_DATE_TIME_H
#define EXESS_SRC_DATE_TIME_H

#include <exess/exess.h>

/**
   Compare two dateTimes.

   Note that local and zoned dateTimes times may not be comparable.  When the
   values aren't comparable, this function arbitrarily chooses the local time
   to be lesser.

   @return Less than, equal to, or greater than zero if `lhs` is less than,
   equal to, or greater than `rhs`, respectively.  Comparable and incomparable
   cases may also be distinguished, see #ExessOrder for details.
*/
EXESS_NONBLOCKING EXESS_CONST_FUNC ExessOrder
exess_compare_date_time(ExessDateTime lhs, ExessDateTime rhs);

#endif // EXESS_SRC_DATE_TIME_H
