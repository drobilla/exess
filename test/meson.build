# Copyright 2021-2022 David Robillard <d@drobilla.net>
# SPDX-License-Identifier: CC0-1.0 OR ISC

autoship = find_program('autoship', required: false)
reuse = find_program('reuse', required: false)

# Add extra warning suppressions for test code
test_args = []
if get_option('strict')
  if cc.get_id() == 'clang'
    test_args = [
      '-Wno-float-equal',
    ]
  elif cc.get_id() == 'gcc'
    test_args = [
      '-Wno-float-equal',
      '-Wno-suggest-attribute=pure',
    ]
  elif cc.get_id() == 'msvc'
    test_args = [
      '/wd4996',  # POSIX name is deprecated
    ]
  endif
endif

# Check project metadata
if autoship.found()
  test('autoship', autoship, args: ['test', exess_src_root], suite: 'data')
endif

# Check licensing metadata
if reuse.found()
  test('REUSE', reuse, args: ['--root', exess_src_root, 'lint'], suite: 'data')
endif

# Public API tests

public_tests = [
  'base64',
  'boolean',
  'byte',
  'canonical',
  'coerce',
  'compare',
  'datatype',
  'date',
  'datetime',
  'decimal',
  'double',
  'duration',
  'float',
  'hex',
  'int',
  'long',
  # 'short', # Built separately below to test arguments
  'strerror',
  'time',
  'timezone',
  'ubyte',
  'uint',
  'ulong',
  'ushort',
  'value',
]

foreach unit : public_tests
  test(unit,
       executable('test_@0@'.format(unit),
                  files('test_@0@.c'.format(unit)),
                  c_args: c_suppressions + test_args,
                  dependencies: [m_dep, exess_dep],
                  include_directories: include_directories('../src')),
       suite: 'public')
endforeach

# Build test_uint as above, but keep the target to test options
test_short = executable('test_short',
                       files('test_short.c'),
                       c_args: c_suppressions + test_args,
                       dependencies: [m_dep, exess_dep],
                       include_directories: include_directories('../src'))

test('short', test_short, suite: 'public')

##########################
# Command-Line Arguments #
##########################

good_args = [
  ['-n', '1024'],
  ['-s', '1234'],
  ['-x'],
]

foreach args : good_args
  name = ' '.join(args).underscorify()
  test(name, test_short, args: args, suite: ['test_args'])
endforeach

bad_args = [
  ['-s'],
  ['-n'],
  ['-X'],
]

foreach args : bad_args
  name = ' '.join(args).underscorify()
  test(name, test_short, args: args, should_fail: true, suite: ['test_args'])
endforeach

######################
# Private Unit Tests #
######################

private_tests = [
  'bigint',
  'int_math',
]

# Find or build a static library to link internal tests with
if get_option('default_library') == 'both'
  libexess_static = libexess.get_static_lib()
elif get_option('default_library') == 'shared'
  libexess_static = static_library(
    meson.project_name() + library_suffix,
    sources,
    include_directories: include_directories(['../include', '../src']),
    c_args: c_suppressions + ['-DEXESS_INTERNAL'],
    dependencies: [m_dep],
    gnu_symbol_visibility: 'default')
else
  libexess_static = libexess
endif

exess_static_dep = declare_dependency(
  include_directories: include_directories(['../include']),
  dependencies: [m_dep],
  link_with: libexess_static)

foreach unit : private_tests
  test(unit,
       executable('test_@0@'.format(unit),
                  files('test_@0@.c'.format(unit)),
                  c_args: c_suppressions + test_args,
                  dependencies: exess_static_dep,
                  include_directories: include_directories('../src')),
       suite: 'private')
endforeach

#######################
# Header Warning Test #
#######################

header_c_args = []
if cc.get_id() == 'clang'
  header_c_args += [
    '-Wno-nullability-extension',
    '-Wno-padded',
  ]
elif cc.get_id() == 'gcc'
  header_c_args += [
    '-Wno-padded',
  ]
endif

test('headers',
     executable('test_headers',
                files('headers/test_headers.c'),
                c_args: header_c_args,
                dependencies: exess_dep,
                include_directories: include_dirs),
    suite: 'public')
