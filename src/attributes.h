// Copyright 2019-2026 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#ifndef EXESS_SRC_ATTRIBUTES_H
#define EXESS_SRC_ATTRIBUTES_H

#if defined(__clang__) && __clang_major__ >= 20
#  define EXESS_I_NONBLOCKING __attribute__((nonblocking))
#else
#  define EXESS_I_NONBLOCKING
#endif

/// A static constant expression usable at compile time
#if ((defined(__STDC_VERSION__) && __STDC_VERSION__ >= 202311L) || \
     (defined(__cplusplus) && __cplusplus >= 201103L))
#  define EXESS_I_CONSTEXPR constexpr
#else
#  define EXESS_I_CONSTEXPR const
#endif

#ifdef __GNUC__
#  define EXESS_I_PURE_FUNC __attribute__((pure))
#  define EXESS_I_CONST_FUNC __attribute__((const))
#else
#  define EXESS_I_PURE_FUNC
#  define EXESS_I_CONST_FUNC
#endif

#endif // EXESS_SRC_ATTRIBUTES_H
