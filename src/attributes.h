// Copyright 2019-2021 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#ifndef EXESS_SRC_ATTRIBUTES_H
#define EXESS_SRC_ATTRIBUTES_H

#ifdef __GNUC__
#  define EXESS_I_PURE_FUNC __attribute__((pure))
#  define EXESS_I_CONST_FUNC __attribute__((const))
#else
#  define EXESS_I_PURE_FUNC
#  define EXESS_I_CONST_FUNC
#endif

#endif // EXESS_SRC_ATTRIBUTES_H
