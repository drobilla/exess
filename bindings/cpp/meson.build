# Copyright 2021-2022 David Robillard <d@drobilla.net>
# SPDX-License-Identifier: CC0-1.0 OR ISC

cpp_headers = files('include/exess/exess.hpp')

exess_hpp_dep = declare_dependency(
  dependencies: exess_dep,
  include_directories: include_directories('include'))

subdir('test')
