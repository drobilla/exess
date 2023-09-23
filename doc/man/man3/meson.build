# Copyright 2021-2023 David Robillard <d@drobilla.net>
# SPDX-License-Identifier: 0BSD OR ISC

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
man3dir = get_option('mandir') / 'man3'
man_docs = custom_target(
  'man3',
  build_by_default: true,
  command: sphinx_build_command + [
    '-b', 'man',
    '-t', 'man',
    sphinx_in_dir,
    '@OUTDIR@',
  ],
  input: [c_rst_files, c_exess_rst, c_index_xml, conf_py],
  install: true,
  install_dir: man3dir,
  output: man_output_names,
)

man_links_mode = get_option('man_links')
if (
  meson.version().version_compare('>=0.61.0')
  and (
    man_links_mode == 'symbolic'
    or (man_links_mode == 'auto'
    and host_machine.system() != 'windows')
  )
)
  # Install symlinks for each man page link if enabled/supported
  foreach target : man_output_names
    foreach link : man_links[target]
      install_symlink(link, pointing_to: target, install_dir: man3dir)
    endforeach
  endforeach

elif man_links_mode != 'disabled'
  # Otherwise, make alias pages for each man page link
  alias_man_in = files('alias.man.in')
  foreach target : man_output_names
    foreach link : man_links[target]
      config = configuration_data()
      config.set('TARGET', 'man3/' + target)
      configure_file(
        configuration: config,
        input: alias_man_in,
        output: link,
        install: true,
        install_dir: man3dir,
      )
    endforeach
  endforeach
endif

if not meson.is_subproject()
  summary('Man pages', get_option('prefix') / man3dir, section: 'Directories')
endif
