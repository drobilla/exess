# Copyright 2021-2023 David Robillard <d@drobilla.net>
# SPDX-License-Identifier: 0BSD OR ISC

html_dir = docdir / versioned_name / 'html'

custom_target(
  'html',
  build_by_default: true,
  command: [
    sphinx_build, '-M', 'html', '@OUTDIR@' / '..', '@OUTDIR@' / '..',
    '-t', 'html',
  ] + sphinx_flags,
  input: [c_rst_files, c_exess_rst, c_index_xml, conf_py],
  install: true,
  install_dir: html_dir,
  output: [
    'genindex.html',
    'index.html',
    'overview.html',
    '_static',
    'api',
  ],
)

if not meson.is_subproject()
  summary('Paginated HTML', get_option('prefix') / html_dir, section: 'Directories')
endif
