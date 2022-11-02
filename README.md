Exess
=====

Exess is a simple library for reading and writing [XSD][] datatypes.

Exess is useful for applications that need to read/write common datatypes
from/to strings, in a standard and locale-independent format.  It supports
reading values from any valid string, and writing values in canonical form.
The implementation is not complete, but includes support for all of the common
generally useful datatypes (the XML-specific and partial Gregorian calendar
datatypes are omitted).

Conversion to a string and back is lossless for all supported values.  For
example, writing a `float` number to a string then reading it back will yield
the exact same `float` as the original value.

The API consists mainly of simple read and write functions for each datatype.
There are also utility functions for rewriting strings in canonical form,
reading/writing binary values to/from opaque buffers,
and coercing values between datatypes.

For flexibility, allocation is handled by the caller, making it possible to
work on the stack, or to read/write values directly from/to fields in a
structure.  Syntax errors are reported with a descriptive error code and
character offset, allowing friendly error messages to be produced.

Supported Datatypes
-------------------

Exess supports reading and writing:

  * `boolean`, like "false", "true", "0", or "1".

  * `decimal`, like "1.234" (stored as `double`).

  * `float` and `double`, like "4.2E1" or "4.2e1".

  * The unbounded integer types `integer`, `nonPositiveInteger`,
    `negativeInteger`, `nonNegativeInteger`, and `nonPositiveInteger` (stored
    as `int64_t` or `uint64_t`).

  * The fixed size integer types `long`, `int`, `short`, `byte`,
    `unsignedLong`, `unsignedInt`, `unsignedShort`, and `unsignedByte`.

  * `duration`, like "P1Y6M".

  * `time`, like "12:30:00.00".

  * `date`, like "2001-12-31".

  * `hex`, like "EC5355".

  * `base64`, like "Zm9vYmFy".

Dependencies
------------

None, except the C standard library.
To be precise, zix uses only the following functions from the standard library:

  * `ldexp`
  * `llrint`
  * `llrintf`
  * `log10`
  * `lrint`
  * `memcmp`
  * `memcpy`
  * `memset`
  * `nextafter`
  * `strcmp`
  * `strncmp`

Building
--------

A [Meson][] build definition is included which can be used to do a proper
system installation with a `pkg-config` file, generate IDE projects, run the
tests, and so on.  For example, the library and tests can be built and run like
so:

    meson setup build
    cd build
    ninja test

See the [Meson documentation][] for more details on using Meson.

Documentation
-------------

 * [API reference (single page)](https://drobilla.gitlab.io/exess/c/singlehtml)
 * [API reference (paginated)](https://drobilla.gitlab.io/exess/c/html)

 -- David Robillard <d@drobilla.net>

[XSD]: https://www.w3.org/TR/xmlschema-2/

[Meson]: https://mesonbuild.com/

[Meson documentation]: https://mesonbuild.com/Quick-guide.html
