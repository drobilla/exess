Exess
=====

Exess is a simple C library for reading and writing [XSD][] datatypes.

Exess provides portable, locale-independent, and standards-backed functions for
converting common numeric and temporal datatypes to and from strings.
Conversions are lossless wherever possible so, for example, a `float` written
to a string will read back as exactly the original value on any system.

The API is lightweight and avoids allocation entirely, making it simple to use
and suitable for constrained contexts like plugins or embedded systems.

Supported Datatypes
-------------------

All of the common generally useful datatypes from the [XSD][] specification are
supported, omitting some XML-specific and partial Gregorian calendar datatypes.
Support for reading and writing is provided for:

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
More precisely, exess uses only the following functions from the standard library:

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
  * Manual
    * Installed as `man exess` and related pages
    * [HTML](https://drobilla.gitlab.io/exess/doc/html/)
    * [Single-page HTML](https://drobilla.gitlab.io/exess/doc/singlehtml/)
    * [EPub](https://drobilla.gitlab.io/exess/doc/Exess-0.0.1.epub)

 -- David Robillard <d@drobilla.net>

[XSD]: https://www.w3.org/TR/xmlschema-2/
