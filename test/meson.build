# Copyright 2021-2022 David Robillard <d@drobilla.net>
# SPDX-License-Identifier: 0BSD OR ISC

# Add extra warning suppressions for test code
test_suppressions = []
if get_option('warning_level') == 'everything'
  if cc.get_id() == 'clang'
    test_suppressions = [
      '-Wno-float-equal',
    ]
  elif cc.get_id() == 'gcc'
    test_suppressions = [
      '-Wno-float-equal',
      '-Wno-suggest-attribute=pure',
    ]
  elif cc.get_id() == 'msvc'
    test_suppressions = [
      '/wd4996', # POSIX name is deprecated
    ]
  endif
endif

if not meson.is_subproject() and get_option('lint')
  # Check project metadata
  autoship = find_program('autoship', required: false)
  if autoship.found()
    test('autoship', autoship, args: ['test', exess_src_root], suite: 'data')
  endif

  # Check licensing metadata
  reuse = find_program('reuse', required: false)
  if reuse.found()
    test(
      'REUSE',
      reuse,
      args: ['--root', exess_src_root, 'lint'],
      suite: 'data',
    )
  endif
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
  'date_time',
  'decimal',
  'double',
  'duration',
  'hex',
  'long',
  'strerror',
  'time',
  'timezone',
  'ubyte',
  'ulong',
  'ushort',
  'value',
]

number_tests = [
  'float',
  'int',
  # 'short', # Built separately below to test arguments
  'uint',
]

foreach unit : public_tests
  test(
    unit,
    executable(
      'test_@0@'.format(unit),
      files('test_@0@.c'.format(unit)),
      c_args: extra_c_args + c_suppressions + test_suppressions,
      dependencies: [m_dep, exess_dep],
    ),
    suite: 'public',
  )
endforeach

foreach unit : number_tests
  test_prog = executable(
    'test_@0@'.format(unit),
    files('test_@0@.c'.format(unit)),
    c_args: extra_c_args + c_suppressions + test_suppressions,
    dependencies: [m_dep, exess_dep],
  )

  # Run default randomized test (like the others run above)
  test(unit + '_random', test_prog, suite: ['public'])

  # Run a short exhaustive test (mainly to hit the test code)
  test(
    unit + '_range',
    test_prog,
    args: ['-x', '-l', '0', '-h', '512'],
    suite: 'public',
  )
endforeach

# Build test_uint as above, but keep the target to test options
test_short = executable(
  'test_short',
  files('test_short.c'),
  c_args: extra_c_args + c_suppressions + test_suppressions,
  dependencies: [m_dep, exess_dep],
)

test('short', test_short, suite: 'public')

##########################
# Command-Line Arguments #
##########################

good_args = [
  ['-n', '1024'],
  ['-s', '1234'],
  ['-x', '-l', '16', '-h', '127'],
  ['-x'],
]

foreach args : good_args
  name = ' '.join(args).underscorify().substring(1)
  test(name, test_short, args: args, suite: ['test_args'])
endforeach

bad_args = [
  ['-X'],
  ['-h'],
  ['-l'],
  ['-n'],
  ['-s'],
]

foreach args : bad_args
  name = ' '.join(args).underscorify().substring(1)
  test(name, test_short, args: args, should_fail: true, suite: ['test_args'])
endforeach

######################
# Private Unit Tests #
######################

if get_option('default_library') == 'shared'
  # Build independent tests with the necessary library source code

  test(
    'int_math',
    executable(
      'test_int_math',
      files('../src/int_math.c', 'test_int_math.c'),
      c_args: extra_c_args + c_suppressions + test_suppressions,
    ),
    suite: 'private',
  )

  test(
    'bigint',
    executable(
      'test_bigint',
      files('../src/bigint.c', '../src/int_math.c', 'test_bigint.c'),
      c_args: extra_c_args + c_suppressions + test_suppressions,
    ),
    suite: 'private',
  )

else
  # Use the main static library because private symbols are available
  if get_option('default_library') == 'both'
    libexess_static = libexess.get_static_lib()
  else
    libexess_static = libexess
  endif

  exess_static_dep = declare_dependency(
    dependencies: [m_dep],
    link_with: libexess_static,
  )

  foreach unit : ['bigint', 'int_math']
    test(
      unit,
      executable(
        'test_@0@'.format(unit),
        files('test_@0@.c'.format(unit)),
        c_args: extra_c_args + c_suppressions + test_suppressions,
        dependencies: exess_static_dep,
      ),
      suite: 'private',
    )
  endforeach
endif

#######################
# Header Warning Test #
#######################

header_suppressions = []
if cc.get_id() in ['clang', 'emscripten']
  header_suppressions += [
    '-Wno-nullability-extension',
    '-Wno-padded',
  ]

  if not meson.is_cross_build()
    header_suppressions += [
      '-Wno-poison-system-directories',
    ]
  endif

elif cc.get_id() == 'gcc'
  header_suppressions += [
    '-Wno-padded',
  ]

elif cc.get_id() == 'msvc'
  header_suppressions += [
    '/experimental:external',
    '/external:W0',
    '/external:anglebrackets',

    '/wd4820', # padding added after data member
  ]
endif

test(
  'headers',
  executable(
    'test_headers',
    files('headers/test_headers.c'),
    c_args: extra_c_args + header_suppressions,
    dependencies: exess_dep,
  ),
  suite: 'public',
)
