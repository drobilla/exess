# Copyright 2021-2023 David Robillard <d@drobilla.net>
# SPDX-License-Identifier: 0BSD OR ISC

docdir = get_option('datadir') / 'doc'

doxygen = find_program('doxygen', required: get_option('docs'))
sphinxygen = find_program('sphinxygen', required: false)
sphinx_build = find_program('sphinx-build', required: get_option('docs'))

if not sphinxygen.found()
  subproject('sphinxygen')
  sphinxygen = find_program('sphinxygen', required: get_option('docs'))
endif

build_docs = doxygen.found() and sphinxygen.found() and sphinx_build.found()

if build_docs
  subdir('c')
endif

if not meson.is_subproject() and meson.version().version_compare('>=0.53.0')
  summary('Documentation', build_docs, bool_yn: true)
endif

