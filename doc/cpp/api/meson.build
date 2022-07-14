# Copyright 2021 David Robillard <d@drobilla.net>
# SPDX-License-Identifier: CC0-1.0 OR ISC

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
