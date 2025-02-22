# Copyright 2021-2025 David Robillard <d@drobilla.net>
# SPDX-License-Identifier: 0BSD OR ISC

config = configuration_data()
config.set('EXESS_SRCDIR', exess_src_root)
config.set('DOX_OUTPUT', meson.current_build_dir() / '..')

c_doxyfile = configure_file(
  configuration: config,
  input: files('../Doxyfile.in'),
  output: 'Doxyfile',
)

c_index_xml = custom_target(
  'index.xml',
  command: [doxygen, '@INPUT0@'],
  input: [c_doxyfile] + c_headers,
  output: [
    'index.xml',

    'combine.xslt',
    'compound.xsd',
    'dir_d44c64559bbebec7f509842c48db8b23.xml',
    'dir_e2a694932a7564946e65c1242ff66514.xml',
    'exess_8h.xml',
    'group__exess.xml',
    'group__exess__base64.xml',
    'group__exess__binary.xml',
    'group__exess__boolean.xml',
    'group__exess__byte.xml',
    'group__exess__canon.xml',
    'group__exess__coercion.xml',
    'group__exess__datatypes.xml',
    'group__exess__date.xml',
    'group__exess__datetime.xml',
    'group__exess__decimal.xml',
    'group__exess__double.xml',
    'group__exess__duration.xml',
    'group__exess__dynamic.xml',
    'group__exess__fixed.xml',
    'group__exess__float.xml',
    'group__exess__hex.xml',
    'group__exess__int.xml',
    'group__exess__long.xml',
    'group__exess__numbers.xml',
    'group__exess__short.xml',
    'group__exess__status.xml',
    'group__exess__symbols.xml',
    'group__exess__time.xml',
    'group__exess__time__and__date.xml',
    'group__exess__timezone.xml',
    'group__exess__ubyte.xml',
    'group__exess__uint.xml',
    'group__exess__ulong.xml',
    'group__exess__ushort.xml',
    'group__exess__value.xml',
    'index.xsd',
    'structExessDate.xml',
    'structExessDateTime.xml',
    'structExessDuration.xml',
    'structExessResult.xml',
    'structExessTime.xml',
    'structExessVariableResult.xml',
    'unionExessValue.xml',
    'xml.xsd',
  ],
)
