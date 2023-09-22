# Copyright 2021-2023 David Robillard <d@drobilla.net>
# SPDX-License-Identifier: 0BSD OR ISC

c_exess_rst = custom_target(
  'exess.rst',
  command: [sphinxygen, '-f', '@INPUT0@', '@OUTDIR@'],
  input: [c_index_xml],
  output: 'exess.rst',
)
