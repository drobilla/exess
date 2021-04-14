project('exess', ['c'],
        version: '0.0.1',
        license: 'ISC',
        meson_version: '>= 0.49.0',
        default_options: [
          'b_ndebug=if-release',
          'buildtype=release',
          'c_std=c99',
          'default_library=shared',
          'warning_level=3',
        ])

exess_src_root = meson.current_source_dir()
major_version = meson.project_version().split('.')[0]
version_suffix = '-@0@'.format(major_version)
versioned_name = 'exess' + version_suffix

# Load build tools
pkg = import('pkgconfig')
cc = meson.get_compiler('c')

# Set ultra strict warnings for developers, if requested
c_warnings = []
c_suppressions = []
if get_option('strict')
  if meson.is_subproject()
    c_warnings = []
  else
    subdir('meson')
    add_project_arguments(all_c_warnings, language: ['c'])
  endif

  if cc.get_id() == 'clang'
    c_suppressions = [
      '-Wno-bad-function-cast',
      '-Wno-c11-extensions',
      '-Wno-nullability-extension',
      '-Wno-padded',
    ]

    if host_machine.system() == 'windows'
      c_suppressions += [
        '-Wno-deprecated-declarations',
      ]
    endif

  elif cc.get_id() == 'gcc'
    c_suppressions = [
      '-Wno-aggregate-return',
      '-Wno-inline',
      '-Wno-padded',
      '-Wno-strict-overflow',
      '-Wno-switch-default',
      '-Wno-unsuffixed-float-constants',
      '-Wno-unused-const-variable',
    ]

  elif cc.get_id() == 'msvc'
    c_suppressions = [
      '/wd4028',  # formal parameter different from declaration
      '/wd4204',  # nonstandard extension: non-constant aggregate initializer
      '/wd4221',  # initialized using address of automatic variable
      '/wd4706',  # assignment within conditional expression
      '/wd4710',  # function not inlined
      '/wd4711',  # function selected for automatic inline expansion
      '/wd4738',  # storing 32-bit float result in memory
      '/wd4820',  # padding added after data member
      '/wd5045',  # will insert Spectre mitigation
    ]
  endif
else
  if cc.get_id() == 'clang'
    c_suppressions = [
      '-Wno-nullability-extension',
    ]
  endif
endif

exess_c_args = cc.get_supported_arguments(c_suppressions)

if cc.get_id() == 'msvc'
  # Suppress warnings in system headers
  add_project_arguments(['/experimental:external',
                         '/external:W0',
                         '/external:anglebrackets'],
                        language: ['c'])
endif

# Detect compiler features

feature_checks = {
  'builtin_clz': 'return __builtin_clz(1);',
  'builtin_clzll': 'return __builtin_clzll(1);',
}

checks = get_option('checks')
foreach name, fragment : feature_checks
  opt = get_option('use_@0@'.format(name))
  code = 'int main(void) { @0@ }'.format(fragment)
  define_name = 'HAVE_@0@'.format(name.to_upper())

  if opt.enabled()
    add_project_arguments(['-D@0@=1'.format(define_name)], language: ['c'])
  elif opt.disabled()
    add_project_arguments(['-D@0@=0'.format(define_name)], language: ['c'])
  elif checks
    if cc.links(code, name: name)
      add_project_arguments(['-D@0@=1'.format(define_name)], language: ['c'])
    else
      add_project_arguments(['-D@0@=0'.format(define_name)], language: ['c'])
    endif
  endif
endforeach

# System libraries
m_dep = cc.find_library('m', required: false)

# Determine library type and the flags needed to build it
if get_option('default_library') == 'both'
  if host_machine.system() == 'windows'
    error('default_library=both is not supported on Windows')
  endif

  library_type = 'both_libraries'
  library_args = ['-DEXESS_INTERNAL']
  prog_args = []
elif get_option('default_library') == 'shared'
  library_type = 'shared_library'
  library_args = ['-DEXESS_INTERNAL']
  prog_args = []
else
  library_type = 'static_library'
  library_args = ['-DEXESS_INTERNAL', '-DEXESS_STATIC']
  prog_args = ['-DEXESS_STATIC']
endif

c_headers = ['include/exess/exess.h']
c_header_files = files(c_headers)

sources = files(
  'src/base64.c',
  'src/bigint.c',
  'src/boolean.c',
  'src/byte.c',
  'src/canonical.c',
  'src/coerce.c',
  'src/datatype.c',
  'src/date.c',
  'src/datetime.c',
  'src/decimal.c',
  'src/digits.c',
  'src/double.c',
  'src/duration.c',
  'src/float.c',
  'src/hex.c',
  'src/int.c',
  'src/int_math.c',
  'src/long.c',
  'src/read_utils.c',
  'src/scientific.c',
  'src/short.c',
  'src/soft_float.c',
  'src/strerror.c',
  'src/strtod.c',
  'src/time.c',
  'src/timezone.c',
  'src/ubyte.c',
  'src/uint.c',
  'src/ulong.c',
  'src/ushort.c',
  'src/variant.c',
  'src/write_utils.c',
  'src/year.c',
)

# Build shared and/or static library/libraries
libexess = build_target(
  versioned_name,
  sources,
  c_args: exess_c_args + library_args,
  dependencies: [m_dep],
  gnu_symbol_visibility: 'hidden',
  include_directories: include_directories(['include']),
  install: true,
  target_type: library_type,
  version: meson.project_version())

# Generage pkg-config file
pkg.generate(
  libexess,
  name: 'Exess',
  filebase: versioned_name,
  subdirs: [versioned_name],
  version: meson.project_version(),
  description: 'A simple and efficient regular expression implementation')

# Install header to a versioned include directory
install_headers(c_header_files, subdir: versioned_name / 'exess')

exess_dep = declare_dependency(
  link_with: libexess,
  include_directories: include_directories(['include']))

if not get_option('docs').disabled()
  subdir('doc')
endif

if get_option('tests')
  subdir('test')
endif

if not meson.is_subproject() and meson.version().version_compare('>=0.53.0')
  summary('Tests', get_option('tests'), bool_yn: true)

  summary('Install prefix', get_option('prefix'))
  summary('Headers', get_option('prefix') / get_option('includedir'))
  summary('Libraries', get_option('prefix') / get_option('libdir'))
endif

