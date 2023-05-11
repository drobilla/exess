# Copyright 2021-2023 David Robillard <d@drobilla.net>
# SPDX-License-Identifier: 0BSD OR ISC

config = configuration_data()
config.set('EXESS_TITLE', get_option('title'))
config.set('EXESS_VERSION', meson.project_version())

conf_py = configure_file(
  configuration: config,
  input: files('../conf.py.in'),
  output: 'conf.py',
)

configure_file(
  copy: true,
  input: files('../summary.rst'),
  output: 'summary.rst',
)

c_rst_files = files(
  'index.rst',
  'overview.rst',
)

foreach f : c_rst_files
  configure_file(copy: true, input: f, output: '@PLAINNAME@')
endforeach

subdir('xml')
subdir('api')

sphinx_flags = ['-E', '-a', '-q']
if get_option('werror')
  sphinx_flags += ['-W']
endif

formats = ['html', 'singlehtml']
foreach format : formats
  if not get_option(format).disabled()
    custom_target(
      format,
      build_by_default: true,
      command: [
        sphinx_build, '-M', format, '@OUTDIR@', '@OUTDIR@', '-t', format,
      ] + sphinx_flags,
      input: [c_rst_files, c_exess_rst, c_index_xml, conf_py],
      install: true,
      install_dir: docdir / versioned_name,
      output: format,
    )
  endif
endforeach

if not get_option('man').disabled()
  subdir('man')
endif
