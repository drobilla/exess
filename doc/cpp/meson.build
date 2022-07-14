# Copyright 2021 David Robillard <d@drobilla.net>
# SPDX-License-Identifier: CC0-1.0 OR ISC

config = configuration_data()
config.set('EXESS_VERSION', meson.project_version())

conf_py = configure_file(configuration: config,
                         input: files('../conf.py.in'),
                         output: 'conf.py')

configure_file(copy: true,
               input: files('../summary.rst'),
               output: 'summary.rst')

cpp_rst_files = files(
  'index.rst',
  'overview.rst',
)

foreach f : cpp_rst_files
  configure_file(copy: true, input: f, output: '@PLAINNAME@')
endforeach

subdir('xml')
subdir('api')

custom_target(
  'singlehtml',
  command: [sphinx_build, '-M', 'singlehtml',
            meson.current_build_dir(), meson.current_build_dir(),
            '-E', '-q', '-t', 'singlehtml'],
  input: [cpp_rst_files, cpp_exess_rst, cpp_index_xml],
  output: 'singlehtml',
  build_by_default: true,
  install: true,
  install_dir: docdir / 'exesspp-0')

custom_target(
  'html',
  command: [sphinx_build, '-M', 'html',
            meson.current_build_dir(), meson.current_build_dir(),
            '-E', '-q', '-t', 'html'],
  input: [cpp_rst_files, cpp_exess_rst, cpp_index_xml],
  output: 'html',
  build_by_default: true,
  install: true,
  install_dir: docdir / 'exesspp-0')
