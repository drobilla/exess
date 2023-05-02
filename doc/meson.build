# Copyright 2021-2023 David Robillard <d@drobilla.net>
# SPDX-License-Identifier: 0BSD OR ISC

docdir = get_option('datadir') / 'doc'

doxygen = find_program('doxygen', required: get_option('docs'))
sphinx_build = find_program('sphinx-build', required: get_option('docs'))

if doxygen.found() and sphinx_build.found()
  sphinxygen = find_program('sphinxygen', required: false)
  if not sphinxygen.found()
    subproject('sphinxygen')
    sphinxygen = find_program('sphinxygen', required: get_option('docs'))
  endif
else
  sphinxygen = disabler()
endif

build_docs = doxygen.found() and sphinxygen.found() and sphinx_build.found()

if build_docs
  pymod = import('python')
  sphinx_lv2_theme_py = pymod.find_installation(
    modules: ['sphinx_lv2_theme'],
    required: false
  )

  if not sphinx_lv2_theme_py.found()
    warning('Missing sphinx_lv2_theme module, falling back to alabaster')
  endif

  subdir('c')
endif

if not meson.is_subproject() and meson.version().version_compare('>=0.53.0')
  summary('Documentation', build_docs, bool_yn: true)
endif
