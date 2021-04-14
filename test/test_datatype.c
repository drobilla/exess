/*
  Copyright 2011-2021 David Robillard <d@drobilla.net>

  Permission to use, copy, modify, and/or distribute this software for any
  purpose with or without fee is hereby granted, provided that the above
  copyright notice and this permission notice appear in all copies.

  THIS SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
  WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
  MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
  ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
  WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
  ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
  OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/

#undef NDEBUG

#include "exess/exess.h"

#include <assert.h>

static void
test_datatype_uris(void)
{
  assert(!exess_datatype_uri(EXESS_NOTHING));
  assert(!exess_datatype_uri((ExessDatatype)(EXESS_BASE64 + 1)));

  for (unsigned i = 1; i <= EXESS_BASE64; ++i) {
    const char* const uri = exess_datatype_uri((ExessDatatype)i);

    assert(uri);
    assert(exess_datatype_from_uri(uri) == (ExessDatatype)i);
  }

  assert(exess_datatype_from_uri(EXESS_XSD_URI) == EXESS_NOTHING);
  assert(exess_datatype_from_uri(EXESS_XSD_URI "unknown") == EXESS_NOTHING);
  assert(exess_datatype_from_uri("garbage") == EXESS_NOTHING);
  assert(exess_datatype_from_uri("http://example.org/very/long/unknown/uri") ==
         EXESS_NOTHING);
}

static void
test_datatype_is_bounded(void)
{
  assert(!exess_datatype_is_bounded(EXESS_NOTHING));
  assert(exess_datatype_is_bounded(EXESS_BOOLEAN));
  assert(!exess_datatype_is_bounded(EXESS_DECIMAL));
  assert(exess_datatype_is_bounded(EXESS_DOUBLE));
  assert(exess_datatype_is_bounded(EXESS_FLOAT));
  assert(!exess_datatype_is_bounded(EXESS_INTEGER));
  assert(!exess_datatype_is_bounded(EXESS_NON_POSITIVE_INTEGER));
  assert(!exess_datatype_is_bounded(EXESS_NEGATIVE_INTEGER));
  assert(exess_datatype_is_bounded(EXESS_LONG));
  assert(exess_datatype_is_bounded(EXESS_INT));
  assert(exess_datatype_is_bounded(EXESS_SHORT));
  assert(exess_datatype_is_bounded(EXESS_BYTE));
  assert(!exess_datatype_is_bounded(EXESS_NON_NEGATIVE_INTEGER));
  assert(exess_datatype_is_bounded(EXESS_ULONG));
  assert(exess_datatype_is_bounded(EXESS_UINT));
  assert(exess_datatype_is_bounded(EXESS_USHORT));
  assert(exess_datatype_is_bounded(EXESS_UBYTE));
  assert(!exess_datatype_is_bounded(EXESS_POSITIVE_INTEGER));
  assert(exess_datatype_is_bounded(EXESS_DURATION));
  assert(exess_datatype_is_bounded(EXESS_DATETIME));
  assert(exess_datatype_is_bounded(EXESS_TIME));
  assert(exess_datatype_is_bounded(EXESS_DATE));
  assert(!exess_datatype_is_bounded(EXESS_HEX));
  assert(!exess_datatype_is_bounded(EXESS_BASE64));
}

int
main(int argc, char** argv)
{
  (void)argv;

  test_datatype_uris();
  test_datatype_is_bounded();

  return argc == 1 ? 0 : 1;
}
