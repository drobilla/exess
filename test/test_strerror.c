// Copyright 2011-2021 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#undef NDEBUG

#include <exess/exess.h>

#include <assert.h>
#include <string.h>

int
main(void)
{
  for (ExessStatus i = EXESS_SUCCESS; i <= EXESS_UNSUPPORTED;
       i             = (ExessStatus)(i + 1)) {
    assert(strlen(exess_strerror(i)) > 0);
  }

  assert(!strcmp(exess_strerror((ExessStatus)-1), "Unknown error"));
  assert(!strcmp(exess_strerror((ExessStatus)9999), "Unknown error"));

  return 0;
}
