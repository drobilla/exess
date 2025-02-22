# Copyright 2021-2025 David Robillard <d@drobilla.net>
# SPDX-License-Identifier: 0BSD OR ISC

singlehtml_dir = docdir / versioned_name / 'singlehtml'

custom_target(
  'singlehtml',
  build_by_default: true,
  command: sphinx_build_command + [
    '-b', 'singlehtml',
    '-t', 'singlehtml',
    sphinx_in_dir,
    '@OUTDIR@',
  ],
  input: [c_rst_files, c_exess_rst, c_index_xml, conf_py],
  install: true,
  install_dir: singlehtml_dir,
  output: [
    '_static',
    'index.html',
    'objects.inv',
  ],
)

if not meson.is_subproject()
  summary(
    'Single HTML',
    get_option('prefix') / singlehtml_dir,
    section: 'Directories',
  )
endif
