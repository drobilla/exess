// Copyright 2019-2026 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#ifndef EXESS_SRC_DURATION_H
#define EXESS_SRC_DURATION_H

#include <exess/exess.h>

/**
   Compare two durations.

   Loosely speaking, a duration is considered less than another if it's a
   shorter duration of time.  However, note that two durations may not be
   comparable, since the relation between fields, such as the number of days in
   a month, varies at different times.  Strictly speaking, two durations are
   comparable if adding them to any `dateTime` would produce results with the
   same ordering.

   The #ExessDuration representation condenses all fields into two values:
   months and seconds.  When the two values are incomparable, a month is
   considered greater than any number of seconds.

   Values with only months, or only seconds, are always comparable.
   Applications are encouraged to always use such values and never carry
   seconds into months or vice-versa.

   @return Less than, equal to, or greater than zero if `lhs` is less than,
   equal to, or greater than `rhs`, respectively.  Comparable and incomparable
   cases may also be distinguished, see #ExessOrder for details.
*/
EXESS_NONBLOCKING EXESS_CONST_FUNC ExessOrder
exess_compare_duration(ExessDuration lhs, ExessDuration rhs);

#endif // EXESS_SRC_DURATION_H
