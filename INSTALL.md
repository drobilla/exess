Installation Instructions
=========================

Prerequisites
-------------

To build from source, you will need:

 * A relatively modern C compiler (GCC, Clang, and MSVC are known to work).

 * [Meson][], which depends on [Python][].

This is a brief overview of building this project with meson.  See the [meson
documentation][] for more detailed information.

Configuration
-------------

The build is configured with the `setup` command, which creates a new build
directory with the given name:

    meson setup build

Some environment variables are read during `setup` and stored with the
configuration:

  * `CC`: Path to C compiler.
  * `CFLAGS`: C compiler options.
  * `LDFLAGS`: Linker options.

However, it's better to use meson options for configuration.  All options can
be inspected with the `configure` command from within the build directory:

    cd build
    meson configure

Options can be set by passing C-style "define" options to `configure`:

    meson configure -Dc_args="-march=native" -Dprefix="/opt/mypackage/"

Building
--------

From within a configured build directory, everything can be built with the
`compile` command:

    meson compile

Similarly, tests can be run with the `test` command:

    meson test

Meson can also generate a project for several popular IDEs, see the `backend`
option for details.

Installation
------------

A compiled project can be installed with the `install` command:

    meson install

You may need to acquire root permissions to install to a system-wide prefix.
For packaging, the installation may be staged to a directory using the
`DESTDIR` environment variable or the `--destdir` option:

    DESTDIR=/tmp/mypackage/ meson install

    meson install --destdir=/tmp/mypackage/

[Meson]: https://mesonbuild.com/

[Python]: http://python.org/

[meson documentation]: https://mesonbuild.com/Quick-guide.html
