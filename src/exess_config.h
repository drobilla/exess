/*
  Copyright 2021 David Robillard <d@drobilla.net>

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

/*
  Configuration header that defines reasonable defaults at compile time.

  This allows compile-time configuration from the command line (typically via
  the build system) while still allowing the source to be built without any
  configuration.  The build system can define EXESS_NO_DEFAULT_CONFIG to disable
  defaults, in which case it must define things like HAVE_FEATURE to enable
  features.  The design here ensures that compiler warnings or
  include-what-you-use will catch any mistakes.
*/

#ifndef EXESS_CONFIG_H
#define EXESS_CONFIG_H

// Define version unconditionally so a warning will catch a mismatch
#define EXESS_VERSION "0.0.1"

#if !defined(EXESS_NO_DEFAULT_CONFIG)

// GCC and clang: __builtin_clz()
#  ifndef HAVE_BUILTIN_CLZ
#    if defined(__has_builtin)
#      if __has_builtin(__builtin_clz)
#        define HAVE_BUILTIN_CLZ 1
#      else
#        define HAVE_BUILTIN_CLZ 0
#      endif
#    elif defined(__GNUC__)
#      define HAVE_BUILTIN_CLZ 1
#    else
#      define HAVE_BUILTIN_CLZ 0
#    endif
#  endif

// GCC and clang: __builtin_clz()
#  ifndef HAVE_BUILTIN_CLZLL
#    if defined(__has_builtin)
#      if __has_builtin(__builtin_clzll)
#        define HAVE_BUILTIN_CLZLL 1
#      else
#        define HAVE_BUILTIN_CLZLL 0
#      endif
#    elif defined(__GNUC__)
#      define HAVE_BUILTIN_CLZLL 1
#    else
#      define HAVE_BUILTIN_CLZLL 0
#    endif
#  endif

#endif // !defined(EXESS_NO_DEFAULT_CONFIG)

/*
  Make corresponding USE_FEATURE defines based on the HAVE_FEATURE defines from
  above or the command line.  The code checks for these using #if (not #ifdef),
  so there will be an undefined warning if it checks for an unknown feature,
  and this header is always required by any code that checks for features, even
  if the build system defines them all.
*/

#define USE_BUILTIN_CLZ HAVE_BUILTIN_CLZ
#define USE_BUILTIN_CLZLL HAVE_BUILTIN_CLZLL

#endif // EXESS_CONFIG_H
