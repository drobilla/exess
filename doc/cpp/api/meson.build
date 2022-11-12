# Copyright 2021 David Robillard <d@drobilla.net>
# SPDX-License-Identifier: 0BSD OR ISC

cpp_exess_rst = custom_target(
  'exess.rst',
  build_by_default: true,
  command: [
    dox_to_sphinx,
    '-l', 'cpp',
    '-f',
    '@INPUT@',
    '@OUTDIR@',
  ],
  input: cpp_index_xml,
  output: 'exess.rst',
)
