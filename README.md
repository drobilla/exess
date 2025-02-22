Exess
=====

Exess is a simple C library for reading and writing values as strings.

It provides portable locale-independent functions for converting basic number,
date, time, and binary datatypes to and from strings.  Conversions are lossless
where possible, so, for example, a `float` written to a string will read back
as exactly the original value on any system.

The supported datatypes, defined by the [XSD][] specification, are explicitly
compatible with many standards (like XML, RDF, and ISO 8601), and incidentally
or partially compatible with many others (like C and JSON).

The library is straightforward to use and has minimal requirements.  It doesn't
depend on an allocator, the current locale, or any other shared mutable state,
making it safe to use in almost any context.

Supported Datatypes
-------------------

Most of the [XSD][] datatypes are implemented,
omitting some XML-specific and recurring Gregorian calendar datatypes.
Support for reading and writing is provided for:

  * `boolean`, like "false", "true", "0", or "1".

  * `decimal`, like "1.234" (stored as `double`).

  * `float` and `double`, like "4.2E1" or "4.2e1".

  * The unbounded integer types `integer`, `nonPositiveInteger`,
    `negativeInteger`, `nonNegativeInteger`, and `nonPositiveInteger` (stored
    as `int64_t` or `uint64_t`).

  * The fixed-size integer types `long`, `int`, `short`, `byte`,
    `unsignedLong`, `unsignedInt`, `unsignedShort`, and `unsignedByte`.

  * `duration`, like "P1Y6M".

  * `dateTime`, like "2001-01-30T14:30:45".

  * `time`, like "12:30:00.00".

  * `date`, like "2001-12-31".

  * `hexBinary`, like "EC5355".

  * `base64Binary`, like "Zm9vYmFy".

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

[XSD]: https://www.w3.org/TR/xmlschema11-2/
