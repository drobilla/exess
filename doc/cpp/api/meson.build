cpp_exess_rst = custom_target(
  'Exess C++ API Sphinx Input',
  command: [dox_to_sphinx,
            '-l', 'cpp',
            '-f', '@INPUT@',
            meson.current_build_dir()],
  input: cpp_index_xml,
  build_by_default: true,
  output: 'exess.rst')
