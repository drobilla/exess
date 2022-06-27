# Copyright 2020-2022 David Robillard <d@drobilla.net>
# SPDX-License-Identifier: CC0-1.0 OR ISC

# Project-specific warning suppressions.
#
# This should be used in conjunction with the generic "warnings" sibling that
# enables all reasonable warnings for the compiler.  It lives here just to keep
# the top-level meson.build more readable.

#####
# C #
#####

if is_variable('cc')
  c_suppressions = []

  if get_option('strict')
    if cc.get_id() == 'emscripten'
      c_suppressions += [
        '-Wno-disabled-macro-expansion',
      ]
    endif

    if cc.get_id() in ['clang', 'emscripten']
      c_suppressions += [
        '-Wno-bad-function-cast',
        '-Wno-c11-extensions',
        '-Wno-padded',
        '-Wno-switch-enum',
      ]

      if host_machine.system() == 'windows'
        c_suppressions += [
          '-Wno-deprecated-declarations',
        ]
      endif

    elif cc.get_id() == 'gcc'
      c_suppressions += [
        '-Wno-inline',
        '-Wno-padded',
        '-Wno-strict-overflow',
        '-Wno-switch-default',
        '-Wno-switch-enum',
        '-Wno-unsuffixed-float-constants',
        '-Wno-unused-const-variable',
      ]

      if host_machine.system() == 'windows'
        c_suppressions += [
          '-Wno-error=format', # MinGW
        ]
      endif

    elif cc.get_id() == 'msvc'
      c_suppressions += [
        '/wd4028', # formal parameter different from declaration
        '/wd4061', # enumerator is not explicitly handled
        '/wd4204', # nonstandard extension: non-constant aggregate initializer
        '/wd4221', # initialized using address of automatic variable
        '/wd4710', # function not inlined
        '/wd4711', # function selected for automatic inline expansion
        '/wd4738', # storing 32-bit float result in memory
        '/wd4820', # padding added after data member
        '/wd5045', # will insert Spectre mitigation
      ]
    endif
  endif

  if cc.get_id() in ['clang', 'emscripten']
    c_suppressions += [
      '-Wno-nullability-extension',
    ]
  endif

  if cc.get_id() == 'msvc'
    c_suppressions += [
      '/wd4706', # assignment within conditional expression
      '/wd4996', # POSIX name for this item is deprecated
    ]
  endif

  c_suppressions = cc.get_supported_arguments(c_suppressions)
endif
