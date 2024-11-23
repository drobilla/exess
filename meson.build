# Copyright 2021-2024 David Robillard <d@drobilla.net>
# SPDX-License-Identifier: 0BSD OR ISC

project(
  'exess',
  ['c'],
  default_options: [
    'b_ndebug=if-release',
    'buildtype=release',
    'c_std=c99',
  ],
  license: 'ISC',
  meson_version: '>= 0.56.0',
  version: '0.0.1',
)

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

# Set global warning suppressions
warning_level = get_option('warning_level')
c_suppressions = []
if cc.get_id() in ['clang', 'emscripten']
  if warning_level == 'everything'
    c_suppressions += [
      '-Wno-declaration-after-statement',
      '-Wno-padded',
      '-Wno-switch-default',
      '-Wno-unsafe-buffer-usage',
    ]

    if not meson.is_cross_build()
      c_suppressions += ['-Wno-poison-system-directories']
    endif
  endif

  if warning_level in ['everything', '3']
    c_suppressions += ['-Wno-nullability-extension']

    if host_machine.system() in ['dragonfly', 'freebsd', 'netbsd', 'openbsd']
      c_suppressions += ['-Wno-c11-extensions']
    endif
  endif

elif cc.get_id() == 'gcc'
  if warning_level == 'everything'
    c_suppressions += [
      '-Wno-padded',
      '-Wno-strict-overflow',
      '-Wno-switch-default',
      '-Wno-unsuffixed-float-constants',
      '-Wno-unused-const-variable',
    ]
  endif

elif cc.get_id() == 'msvc'
  if warning_level == 'everything'
    c_suppressions += [
      '/wd4464', # relative include path contains '..'
      '/wd4710', # function not inlined
      '/wd4711', # function selected for automatic inline expansion
      '/wd4820', # padding added after data member
      '/wd5045', # will insert Spectre mitigation
    ]
  endif
endif

c_suppressions = cc.get_supported_arguments(c_suppressions)

##########################
# Platform Configuration #
##########################

# Use versioned name everywhere to support parallel major version installations
if host_machine.system() == 'windows'
  if get_option('default_library') == 'both'
    error('default_library=both is not supported on Windows')
  endif
  soversion = ''
else
  soversion = meson.project_version().split('.')[0]
endif

# Build platform-specific configuration arguments
platform_c_args = []
if get_option('checks').disabled()
  # Generic build without platform-specific features
  platform_c_args += ['-DEXESS_NO_DEFAULT_CONFIG']
elif get_option('checks').auto()
  # Statically detect configuration from the build environment
  platform_c_args += system_c_args
else
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
    elif cc.links(code, name: name)
      add_project_arguments(['-D@0@=1'.format(define_name)], language: ['c'])
    else
      add_project_arguments(['-D@0@=0'.format(define_name)], language: ['c'])
    endif
  endforeach
endif

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
  'src/date_utils.c',
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
extra_c_args = []
if get_option('default_library') == 'static'
  extra_c_args = ['-DEXESS_STATIC']
endif

# Build shared and/or static library
libexess = library(
  versioned_name,
  sources,
  c_args: platform_c_args + extra_c_args + c_suppressions + ['-DEXESS_INTERNAL'],
  darwin_versions: [major_version + '.0.0', meson.project_version()],
  dependencies: [m_dep],
  gnu_symbol_visibility: 'hidden',
  include_directories: include_dirs,
  install: true,
  soversion: soversion,
  version: meson.project_version(),
)

# Declare dependency for internal meson dependants
exess_dep = declare_dependency(
  compile_args: extra_c_args,
  include_directories: include_dirs,
  link_with: libexess,
)

# Generage pkg-config file for external dependants
pkg.generate(
  libexess,
  description: 'Simple library for reading and writing XSD datatypes',
  extra_cflags: extra_c_args,
  filebase: versioned_name,
  name: 'Exess',
  subdirs: [versioned_name],
  version: meson.project_version(),
)

# Override pkg-config dependency for internal meson dependants
meson.override_dependency(versioned_name, exess_dep)

# Install header to a versioned include directory
install_headers(c_headers, subdir: versioned_name / 'exess')

# Display top-level summary (augmented in subdirectories)
if not meson.is_subproject()
  summary(
    'Tests',
    not get_option('tests').disabled(),
    bool_yn: true,
    section: 'Configuration',
  )
  summary('Install prefix', get_option('prefix'), section: 'Directories')
  summary(
    'Headers',
    get_option('prefix') / get_option('includedir'),
    section: 'Directories',
  )
  summary(
    'Libraries',
    get_option('prefix') / get_option('libdir'),
    section: 'Directories',
  )
endif

#######################
# Optional Components #
#######################

if not get_option('docs').disabled()
  subdir('doc')
endif

if not get_option('tests').disabled()
  subdir('test')
endif
