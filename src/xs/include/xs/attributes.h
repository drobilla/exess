// Copyright 2019-2025 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#ifndef XS_ATTRIBUTES_H
#define XS_ATTRIBUTES_H

#ifdef __GNUC__
#  define XS_PURE_FUNC __attribute__((pure))
#  define XS_CONST_FUNC __attribute__((const))
#else
#  define XS_PURE_FUNC
#  define XS_CONST_FUNC
#endif

#endif // XS_ATTRIBUTES_H
