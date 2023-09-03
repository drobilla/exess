# Copyright 2021-2023 David Robillard <d@drobilla.net>
# SPDX-License-Identifier: 0BSD OR ISC

make_man_link = find_program('../../scripts/make_man_link.py')

# Generated man pages to any links/aliases to create for that page
man_links = {
  'exess.3': [],
  'exess_base64.3': [
    'exess_base64_decoded_size.3',
    'exess_read_base64.3',
    'exess_write_base64.3',
  ],

  'exess_boolean.3': ['exess_read_boolean.3', 'exess_write_boolean.3'],
  'exess_byte.3': ['exess_read_byte.3', 'exess_write_byte.3'],
  'exess_canon.3': ['exess_write_canonical.3'],

  'exess_datatypes.3': [
    'ExessDatatype.3',
    'exess_datatype_from_uri.3',
    'exess_datatype_is_bounded.3',
    'exess_datatype_uri.3',
    'exess_max_length.3',
    'exess_value_size.3',
  ],

  'exess_date.3': [
    'ExessDate.3',
    'exess_date_compare.3',
    'exess_read_date.3',
    'exess_write_date.3',
  ],

  'exess_datetime.3': [
    'ExessDateTime.3',
    'exess_datetime_compare.3',
    'exess_add_datetime_duration.3',
    'exess_read_datetime.3',
    'exess_write_datetime.3',
  ],

  'exess_decimal.3': ['exess_read_decimal.3', 'exess_write_decimal.3'],
  'exess_double.3': ['exess_read_double.3', 'exess_write_double.3'],

  'exess_duration.3': [
    'ExessDuration.3',
    'exess_duration_compare.3',
    'exess_read_duration.3',
    'exess_write_duration.3',
  ],

  'exess_float.3': ['exess_read_float.3', 'exess_write_float.3'],

  'exess_hex.3': [
    'exess_hex_decoded_size.3',
    'exess_read_hex.3',
    'exess_write_hex.3',
  ],

  'exess_int.3': ['exess_read_int.3', 'exess_write_int.3'],
  'exess_long.3': ['exess_read_long.3', 'exess_write_long.3'],
  'exess_short.3': ['exess_read_short.3', 'exess_write_short.3'],
  'exess_status.3': ['ExessResult.3', 'ExessStatus.3', 'exess_strerror.3'],
  'exess_symbols.3': ['EXESS_XSD_URI.3'],

  'exess_time.3': [
    'ExessTime.3',
    'exess_time_compare.3',
    'exess_read_time.3',
    'exess_write_time.3',
  ],

  'exess_timezone.3': ['ExessTimezone.3'],
  'exess_ubyte.3': ['exess_read_ubyte.3', 'exess_write_ubyte.3'],
  'exess_uint.3': ['exess_read_uint.3', 'exess_write_uint.3'],
  'exess_ulong.3': ['exess_read_ulong.3', 'exess_write_ulong.3'],
  'exess_ushort.3': ['exess_read_ushort.3', 'exess_write_ushort.3'],

  'exess_value.3': [
    'ExessCoercion.3',
    'ExessCoercions.3',
    'ExessValue.3',
    'exess_value_compare.3',
    'exess_value_coerce.3',
    'exess_read_value.3',
    'exess_write_value.3',
  ],
}

# Run Sphinx to generate man pages
man_output_names = man_links.keys()
man_docs = custom_target(
  'man',
  build_by_default: true,
  command: sphinx_build_command + [
    '-b', 'man',
    '-t', 'man',
    sphinx_in_dir,
    '@OUTDIR@',
  ],
  input: [c_rst_files, c_exess_rst, c_index_xml, conf_py],# + files('exess.rst'),
  install: true,
  install_dir: get_option('mandir') / 'man3',
  output: man_output_names,
)

# Assemble a command and target prefix for creating link/alias pages
make_man_link_command = [make_man_link]
man_link_prefix = 'man3/'

# Make an alias man page or symlink for each link
target_index = 0
foreach target : man_output_names
  foreach link : man_links[target]
    custom_target(
      link,
      build_by_default: true,
      command: make_man_link_command + [man_link_prefix + target, '@OUTPUT@'],
      input: man_docs[target_index],
      install: true,
      install_dir: get_option('mandir') / 'man3',
      output: link,
    )
  endforeach

  target_index = target_index + 1
endforeach

if not meson.is_subproject()
  summary('Man pages', get_option('prefix') / get_option('mandir') / 'man3', section: 'Directories')
endif
