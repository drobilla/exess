# Copyright 2021-2025 David Robillard <d@drobilla.net>
# SPDX-License-Identifier: 0BSD OR ISC

html_dir = docdir / versioned_name / 'html'

custom_target(
  'html',
  build_by_default: true,
  command: sphinx_build_command + [
    '-b', 'html',
    '-t', 'html',
    sphinx_in_dir,
    '@OUTDIR@',
  ],
  input: [c_rst_files, c_exess_rst, c_index_xml, conf_py],
  install: true,
  install_dir: html_dir,
  output: [
    '_static',
    'api',
    'description.html',
    'genindex.html',
    'index.html',
    'objects.inv',
    'search.html',
    'searchindex.js',
  ],
)

if not meson.is_subproject()
  summary(
    'HTML',
    get_option('prefix') / html_dir,
    section: 'Directories',
  )
endif
