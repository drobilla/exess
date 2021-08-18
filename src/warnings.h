// Copyright 2019-2021 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

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
