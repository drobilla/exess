add_languages(['cpp'])
cpp = meson.get_compiler('cpp')

# Set ultra strict warnings for developers, if requested
cpp_suppressions = []
if get_option('strict')
  if cpp.get_id() == 'clang'
    cpp_suppressions = [
      '-Wno-c++98-compat',
      '-Wno-c++98-compat-pedantic',
      '-Wno-documentation-unknown-command',
      '-Wno-nullability-extension',
      '-Wno-padded',
    ]

  elif cpp.get_id() == 'gcc'
    cpp_suppressions = [
      '-Wno-inline',
      '-Wno-padded',
      '-Wno-switch-default',
      '-Wno-unused-const-variable',
      '-Wno-useless-cast',
    ]

  elif cpp.get_id() == 'msvc'
    cpp_suppressions = [
      '/wd4028',  # formal parameter different from declaration
      '/wd4204',  # nonstandard extension: non-constant aggregate initializer
      '/wd4706',  # assignment within conditional expression
      '/wd4710',  # function not inlined
      '/wd4711',  # function selected for automatic inline expansion
      '/wd4820',  # padding added after data member
      '/wd5045',  # will insert Spectre mitigation
    ]
  endif
else
  if cpp.get_id() == 'clang'
    cpp_suppressions = [
      '-Wno-nullability-extension',
    ]
  endif
endif

exess_cpp_args = cpp.get_supported_arguments(cpp_suppressions)

cpp_headers = ['include/exess/exess.hpp']
cpp_header_files = files(cpp_headers)

exess_hpp_dep = declare_dependency(
  dependencies: exess_dep,
  include_directories: include_directories('include'))

subdir('test')
