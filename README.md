Exess
=====

Exess is a simple library for reading and writing [XSD][] datatypes.

Exess is useful for applications that need to read and write common datatypes
as strings in a standard and locale-independent format.  It supports reading
values from any valid string, and writing values in canonical form.  The
implementation is not complete, but includes support for all of the common
generally useful datatypes (the XML-specific and partial Gregorian calendar
datatypes are omitted).

Conversion to a string and back is lossless for all supported values.  For
example, writing a `float` number to a string then reading it back will yield
the exact same `float` as the original value.

The API consists mainly of simple read and write functions for each datatype.
There are also utility functions for rewriting strings in canonical form,
reading/writing opaque binary values,
and coercing values between datatypes.

For flexibility, allocation is handled by the caller, making it possible to
work on the stack, or to directly read/write fields in a structure.  Syntax
errors are reported with a character offset and error code, allowing the caller
to produce precise and friendly error messages.

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

  * `datetime`, like "2001-01-30T14:30:45".

  * `time`, like "12:30:00.00".

  * `date`, like "2001-12-31".

  * `hex`, like "EC5355".

  * `base64`, like "Zm9vYmFy".

Dependencies
------------

None, except the C standard library.
To be precise, exess uses only the following functions from the standard library:

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
  * `trunc`
  * `truncf`

Documentation
-------------

 * [Installation instructions](INSTALL.md)
 * [API reference (single page)](https://drobilla.gitlab.io/exess/doc/singlehtml/)
 * [API reference (paginated)](https://drobilla.gitlab.io/exess/doc/html/)
 * [API reference (EPub)](https://drobilla.gitlab.io/exess/doc/Exess-0.0.1.epub)

 -- David Robillard <d@drobilla.net>

[XSD]: https://www.w3.org/TR/xmlschema-2/

[Meson]: https://mesonbuild.com/

[Meson documentation]: https://mesonbuild.com/Quick-guide.html
