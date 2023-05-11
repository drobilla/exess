# Copyright 2021-2023 David Robillard <d@drobilla.net>
# SPDX-License-Identifier: 0BSD OR ISC

epub_dir = docdir / versioned_name / 'epub'

custom_target(
  'epub',
  build_by_default: true,
  command: [
    sphinx_build, '-M', 'epub', '@OUTDIR@' / '..', '@OUTDIR@' / '..',
    '-t', 'epub',
  ] + sphinx_flags,
  input: [c_rst_files, c_exess_rst, c_index_xml, conf_py],
  install: true,
  install_dir: epub_dir,
  output: ['Exess-@0@.epub'.format(meson.project_version())],
  # output: [
  #   'genindex.epub',
  #   'index.epub',
  #   'overview.epub',
  #   '_static',
  #   'api',
  # ],
)

if not meson.is_subproject()
  summary('EPub', get_option('prefix') / epub_dir, section: 'Directories')
endif
