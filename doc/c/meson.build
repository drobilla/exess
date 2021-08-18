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

c_rst_files = files(
  'index.rst',
  'overview.rst',
)

foreach f : c_rst_files
  configure_file(copy: true, input: f, output: '@PLAINNAME@')
endforeach

subdir('xml')
subdir('api')

custom_target(
  'Exess C API Documentation (singlehtml)',
  command: [sphinx_build, '-M', 'singlehtml',
            meson.current_build_dir(), meson.current_build_dir(),
            '-E', '-q', '-t', 'singlehtml'],
  input: [c_rst_files, c_exess_rst, c_index_xml],
  output: 'singlehtml',
  build_by_default: true,
  install: true,
  install_dir: docdir / 'exess-0')

custom_target(
  'Exess C API Documentation (html)',
  command: [sphinx_build, '-M', 'html',
            meson.current_build_dir(), meson.current_build_dir(),
            '-E', '-q', '-t', 'html'],
  input: [c_rst_files, c_exess_rst, c_index_xml],
  output: 'html',
  build_by_default: true,
  install: true,
  install_dir: docdir / 'exess-0')

# custom_target(
#   'man pages for exess',
#   command: [sphinx_build, '-M', 'man',
#             meson.current_build_dir(), meson.current_build_dir(),
#             '-E', '-q', '-t', 'man'],
#   input: [c_rst_files, c_exess_rst, c_index_xml],
#   output: ['man/exess_base64.3'],
#   build_by_default: true)

subdir('man')
