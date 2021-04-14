test('exess_hpp',
     executable('test_exess_hpp',
                files('test_exess_hpp.cpp'),
                cpp_args: exess_cpp_args + prog_args,
                dependencies: [exess_hpp_dep],
                include_directories: include_directories('../../../test',
                                                         '../../../src')),
     suite: 'cpp')
