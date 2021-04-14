config = configuration_data()
config.set('EXESS_SRCDIR', exess_src_root)
config.set('DOX_OUTPUT', meson.current_build_dir() / '..')

cpp_doxyfile = configure_file(configuration: config,
                              input: files('../Doxyfile.in'),
                              output: 'Doxyfile')

cpp_index_xml = custom_target(
  'exess-cpp-index.xml',
  command: [doxygen, '@INPUT0@'],
  input: [cpp_doxyfile] + c_header_files + cpp_header_files,
  output: 'index.xml')

