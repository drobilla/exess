Installation
============

This is a brief overview of building and installing from source on the command
line using [meson][].  Building requires a relatively modern C compiler (GCC,
Clang, and MSVC are tested), meson, and its dependencies [Python][] and (by
default) ninja.  It can also generate projects for popular IDEs, see the
[`meson configure --backend`](https://mesonbuild.com/Commands.html#configure)
option for details.

Configuring
-----------

The build is configured with the `setup` command, which creates a new build
directory with the given name:

    meson setup build

The environment variables `CC` and `CC_LD`, if set, give the C compiler and
linker during setup, but otherwise it's best to configure via meson options.
After setup, enter the build directory (and stay):

    cd build

From here, the build configuration can be inspected:

    meson configure

Options can be set by passing C-style "define" options to `configure`:

    meson configure -Dc_args="-march=native" -Dprefix="/opt/mypackage/"

Building
--------

Once configured, the `compile` command will build everything, and the `test`
command will run all tests:

    meson compile
    meson test

Installing
----------

A compiled project can be installed with the `install` command:

    meson install

You may need to acquire root permissions to install to a system-wide prefix.
For packaging, the installation may be staged to a directory using the
`DESTDIR` environment variable or the `meson install --destdir` option:

    DESTDIR=/tmp/mypackage/ meson install
    meson install --destdir=/tmp/mypackage/

[meson]: https://mesonbuild.com/

[Python]: http://python.org/
