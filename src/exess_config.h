// Copyright 2021-2023 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

/*
  Configuration header that defines reasonable defaults at compile-time.

  This allows configuration from the command-line (usually by the build system)
  while still allowing the code to compile "as-is" with reasonable default
  features on supported platforms.

  This system is designed so that, ideally, no command-line or build-system
  configuration is needed, but automatic feature detection can be disabled or
  overridden for maximum control.  It should never be necessary to edit the
  source code to achieve a given configuration.

  Usage:

  - By default, features are enabled if they can be detected or assumed to be
    available from the build environment, unless `EXESS_NO_DEFAULT_CONFIG` is
    defined, which disables everything by default.

  - If a symbol like `HAVE_SOMETHING` is defined to non-zero, then the
    "something" feature is assumed to be available.

  Code rules:

  - To check for a feature, this header must be included, and the symbol
    `USE_SOMETHING` used as a boolean in an `#if` expression.

  - None of the other configuration symbols described here may be used
    directly.  In particular, this header should be the only place in the
    source code that touches `HAVE` symbols.
*/

#ifndef EXESS_CONFIG_H
#define EXESS_CONFIG_H

// Define version unconditionally so a warning will catch a mismatch
#define EXESS_VERSION "0.0.1"

#if !defined(EXESS_NO_DEFAULT_CONFIG)

// GCC and clang: __builtin_clz()
#  if !defined(HAVE_BUILTIN_CLZ)
#    if defined(__has_builtin)
#      if __has_builtin(__builtin_clz)
#        define HAVE_BUILTIN_CLZ 1
#      endif
#    elif defined(__GNUC__)
#      define HAVE_BUILTIN_CLZ 1
#    endif
#  endif

// GCC and clang: __builtin_clz()
#  if !defined(HAVE_BUILTIN_CLZLL)
#    if defined(__has_builtin)
#      if __has_builtin(__builtin_clzll)
#        define HAVE_BUILTIN_CLZLL 1
#      endif
#    elif defined(__GNUC__)
#      define HAVE_BUILTIN_CLZLL 1
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

#if defined(HAVE_BUILTIN_CLZ) && HAVE_BUILTIN_CLZ
#  define USE_BUILTIN_CLZ 1
#else
#  define USE_BUILTIN_CLZ 0
#endif

#if defined(HAVE_BUILTIN_CLZLL) && HAVE_BUILTIN_CLZLL
#  define USE_BUILTIN_CLZLL 1
#else
#  define USE_BUILTIN_CLZLL 0
#endif

#endif // EXESS_CONFIG_H
