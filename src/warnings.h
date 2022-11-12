/*
  Copyright 2019-2021 David Robillard <d@drobilla.net>

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

#ifndef EXESS_WARNINGS_H
#define EXESS_WARNINGS_H

#if defined(__clang__)

#  define EXESS_DISABLE_CONVERSION_WARNINGS              \
    _Pragma("clang diagnostic push")                     \
    _Pragma("clang diagnostic ignored \"-Wconversion\"") \
    _Pragma("clang diagnostic ignored \"-Wdouble-promotion\"")

#  define EXESS_RESTORE_WARNINGS _Pragma("clang diagnostic pop")

#elif defined(__GNUC__)

#  define EXESS_DISABLE_CONVERSION_WARNINGS                  \
    _Pragma("GCC diagnostic push")                           \
    _Pragma("GCC diagnostic ignored \"-Wconversion\"")       \
    _Pragma("GCC diagnostic ignored \"-Wfloat-conversion\"") \
    _Pragma("GCC diagnostic ignored \"-Wdouble-promotion\"")

#  define EXESS_RESTORE_WARNINGS _Pragma("GCC diagnostic pop")

#else

#  define EXESS_DISABLE_CONVERSION_WARNINGS
#  define EXESS_RESTORE_WARNINGS

#endif

#endif // EXESS_WARNINGS_H
