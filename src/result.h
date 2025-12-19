// Copyright 2019-2025 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#ifndef EXESS_SRC_RESULT_H
#define EXESS_SRC_RESULT_H

#ifdef __cplusplus
#  define RESULT(s, c) (ExessResult{s, c})
#  define VRESULT(s, r, w) (ExessVariableResult{s, r, w})
#else
#  define RESULT(s, c) ((ExessResult){s, c})
#  define VRESULT(s, r, w) ((ExessVariableResult){s, r, w})
#endif

#endif // EXESS_SRC_RESULT_H
