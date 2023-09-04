# Copyright 2021-2023 David Robillard <d@drobilla.net>
# SPDX-License-Identifier: 0BSD OR ISC

docdir = get_option('datadir') / 'doc'

# Find required programs
doxygen = find_program('doxygen', required: get_option('docs'))
sphinx_build = find_program('sphinx-build', required: get_option('docs'))

# Find sphinxygen or fall back to subproject
sphinxygen = disabler()
if doxygen.found() and sphinx_build.found()
  sphinxygen = find_program('sphinxygen', required: false)
  if not sphinxygen.found()
    subproject('sphinxygen')
    sphinxygen = find_program('sphinxygen', required: get_option('docs'))
  endif
endif

# Build documentation if all required tools are found
build_docs = doxygen.found() and sphinx_build.found() and sphinxygen.found()
if not meson.is_subproject()
  summary('Documentation', build_docs, bool_yn: true, section: 'Configuration')
endif

if build_docs
  subdir('_static')

  # Warn if the "official" theme isn't present
  pymod = import('python')
  doc_modules = ['sphinx_lv2_theme']
  if not pymod.find_installation('python3', modules: doc_modules, required: false).found()
    warning('Missing sphinx_lv2_theme module, falling back to alabaster')
  endif

  # Configure conf.py for Sphinx
  config = configuration_data()
  config.set('EXESS_STATIC_PATH', doc_static_build_dir)
  config.set('EXESS_TITLE', get_option('title'))
  config.set('EXESS_VERSION', meson.project_version())
  conf_py = configure_file(
    configuration: config,
    input: files('conf.py.in'),
    output: 'conf.py',
  )

  # Copy hand-written documentation files
  c_rst_files = files('index.rst', 'description.rst')
  foreach f : c_rst_files
    configure_file(copy: true, input: f, output: '@PLAINNAME@')
  endforeach

  # Generate reference documentation input with Doxygen and Sphinxygen
  subdir('xml')
  subdir('api')

  # Build strict Sphinx flags, with termination on warnings if werror=true
  sphinx_in_dir = meson.current_build_dir()
  sphinx_flags = ['-E', '-a', '-q']
  if get_option('werror')
    sphinx_flags += ['-W']
  endif

  # Run Sphinx to generate final documentation for each format
  sphinx_build_command = [sphinx_build] + sphinx_flags
  foreach format : ['epub', 'html', 'man', 'singlehtml']
    if not get_option(format).disabled()
      subdir(format)
    endif
  endforeach
endif

