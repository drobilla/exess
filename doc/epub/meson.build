# Copyright 2021-2023 David Robillard <d@drobilla.net>
# SPDX-License-Identifier: 0BSD OR ISC

epub_dir = docdir / versioned_name / 'epub'
epub_name = 'Exess-@0@.epub'.format(meson.project_version())

custom_target(
  epub_name,
  build_by_default: true,
  command: sphinx_build_command + [
    '-b', 'epub',
    '-t', 'epub',
    '-d', sphinx_in_dir / '.doctrees',
    sphinx_in_dir,
    '@OUTDIR@',
  ],
  input: [c_rst_files, c_exess_rst, c_index_xml, conf_py],
  install: true,
  install_dir: epub_dir,
  output: epub_name,
)

if not meson.is_subproject()
  summary('EPub', get_option('prefix') / epub_dir, section: 'Directories')
endif
