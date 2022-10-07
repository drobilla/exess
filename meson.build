# Copyright 2021-2022 David Robillard <d@drobilla.net>
# SPDX-License-Identifier: CC0-1.0 OR ISC

project('exess', ['c'],
        version: '0.0.1',
        license: 'ISC',
        meson_version: '>= 0.53.0',
        default_options: [
          'b_ndebug=if-release',
          'buildtype=release',
          'c_std=c99',
        ])

exess_src_root = meson.current_source_dir()
major_version = meson.project_version().split('.')[0]
version_suffix = '-@0@'.format(major_version)
versioned_name = 'exess' + version_suffix

#######################
# Compilers and Flags #
#######################

# Required tools
pkg = import('pkgconfig')
cc = meson.get_compiler('c')

# Set global warning flags
if get_option('strict') and not meson.is_subproject()
  subdir('meson/warnings')
  add_project_arguments(all_c_warnings, language: ['c'])
endif
subdir('meson/suppressions')

#####################
# Compiler Features #
#####################

feature_checks = {
  'builtin_clz': 'return __builtin_clz(1);',
  'builtin_clzll': 'return __builtin_clzll(1);',
}

foreach name, fragment : feature_checks
  opt = get_option('use_@0@'.format(name))
  code = 'int main(void) { @0@ }'.format(fragment)
  define_name = 'HAVE_@0@'.format(name.to_upper())

  if opt.enabled()
    add_project_arguments(['-D@0@=1'.format(define_name)], language: ['c'])
  elif opt.disabled()
    add_project_arguments(['-D@0@=0'.format(define_name)], language: ['c'])
  elif get_option('checks')
    if cc.links(code, name: name)
      add_project_arguments(['-D@0@=1'.format(define_name)], language: ['c'])
    else
      add_project_arguments(['-D@0@=0'.format(define_name)], language: ['c'])
    endif
  endif
endforeach

################
# Dependencies #
################

m_dep = cc.find_library('m', required: false)

###########
# Library #
###########

include_dirs = include_directories(['include'])
c_headers = files('include/exess/exess.h')

sources = files(
  'src/base64.c',
  'src/bigint.c',
  'src/boolean.c',
  'src/byte.c',
  'src/canonical.c',
  'src/coerce.c',
  'src/compare.c',
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
  'src/value.c',
  'src/write_utils.c',
  'src/year.c',
)

# Set appropriate arguments for building against the library type
subdir('meson/library')
if get_option('default_library') == 'static_library'
  add_project_arguments(['-DEXESS_STATIC'], language: ['c'])
endif

# Build shared and/or static library
libexess = library(
  meson.project_name() + library_suffix,
  sources,
  c_args: c_suppressions + ['-DEXESS_INTERNAL'],
  dependencies: [m_dep],
  gnu_symbol_visibility: 'hidden',
  include_directories: include_dirs,
  install: true,
  version: meson.project_version(),
)

# Generage pkg-config file for external dependants
pkg.generate(
  libexess,
  description: 'Simple library for reading and writing XSD datatypes',
  filebase: versioned_name,
  name: 'Exess',
  subdirs: [versioned_name],
  version: meson.project_version(),
)

# Declare dependency for internal meson dependants
exess_dep = declare_dependency(
  include_directories: include_dirs,
  link_with: libexess,
)

# Override pkg-config dependency for internal meson dependants
meson.override_dependency(versioned_name, exess_dep)

# Install header to a versioned include directory
install_headers(c_headers, subdir: versioned_name / 'exess')

#######################
# Optional Components #
#######################

if not get_option('docs').disabled()
  subdir('doc')
endif

if not get_option('tests').disabled()
  subdir('test')
endif

if not meson.is_subproject()
  summary('Tests', not get_option('tests').disabled(), bool_yn: true)
  summary('Install prefix', get_option('prefix'))
  summary('Headers', get_option('prefix') / get_option('includedir'))
  summary('Libraries', get_option('prefix') / get_option('libdir'))
endif
