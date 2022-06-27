# Copyright 2021-2022 David Robillard <d@drobilla.net>
# SPDX-License-Identifier: CC0-1.0 OR ISC

test_args = []
test_link_args = []
if cpp.get_id() == 'emscripten'
  test_args += ['-s', 'DISABLE_EXCEPTION_CATCHING=0']
  test_link_args += ['-s', 'DISABLE_EXCEPTION_CATCHING=0']
elif cpp.get_id() == 'gcc'
  test_args += [
    '-Wno-shadow',
  ]
endif

test('exess_hpp',
     executable('test_exess_hpp',
                files('test_exess_hpp.cpp'),
                cpp_args: cpp_suppressions + test_args,
                link_args: test_link_args,
                dependencies: [exess_hpp_dep],
                include_directories: include_directories('../../../test',
                                                         '../../../src')),
     suite: 'cpp')
