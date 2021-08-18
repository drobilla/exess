# Copyright 2021 David Robillard <d@drobilla.net>
# SPDX-License-Identifier: CC0-1.0 OR ISC

test('exess_hpp',
     executable('test_exess_hpp',
                files('test_exess_hpp.cpp'),
                cpp_args: exess_cpp_args + prog_args,
                dependencies: [exess_hpp_dep],
                include_directories: include_directories('../../../test',
                                                         '../../../src')),
     suite: 'cpp')
