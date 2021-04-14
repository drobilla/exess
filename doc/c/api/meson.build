c_exess_rst = custom_target(
  'Exess C API Sphinx Input',
  command: [dox_to_sphinx, '-f', '@INPUT0@', meson.current_build_dir()],
  input: [c_index_xml] + c_rst_files,
  output: 'exess.rst')
