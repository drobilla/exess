Exess
=====

Exess is a simple C library for reading and writing values as strings.

It provides portable locale-independent functions for converting common number,
date, time, and binary datatypes.  Values are preserved exactly wherever
possible, so, for example, a `float` written to a string will be read back as
exactly the original value on any system.

The supported datatypes, defined by the [XSD][] specification, are explicitly
compatible with many standards (like XML, RDF, and ISO 8601), and most are
incidentally compatible with many others (like C and JSON).

The library is straightforward to use and doesn't depend on an allocator, the
current locale, or any other shared mutable state, making it safe to use in
almost any context.

Supported Datatypes
-------------------

Most of the [XSD][] datatypes are implemented, omitting some XML-specific and
recurring Gregorian calendar datatypes:

* `boolean`, like "false", "true", "0", or "1".

* `decimal`, like "1.234".

* `float` and `double`, like "4.2E1" or "4.2e1".

* The unbounded integers `integer`, `nonPositiveInteger`, `negativeInteger`,
  `nonNegativeInteger`, and `nonPositiveInteger`, like "56" or "-78".

* The constrained integers `long`, `int`, `short`, `byte`, `unsignedLong`,
  `unsignedInt`, `unsignedShort`, and `unsignedByte`, like "-90" or "12".

* `duration`, like "P1Y2M3DT4H5M", and its totally-ordered derivatives
  `yearMonthDuration` (like "P6Y7M") and `dayTimeDuration` (like "P8DT9H").

* `dateTime`, like "2001-01-30T14:30:45", and its totally-ordered derivative
  `dateTimeStamp` (like "1929-10-24T09:30:00-04:00").

* `time`, like "12:30:00.000000001".

* `date`, like "2001-12-31".

* `hexBinary`, like "EC5355".

* `base64Binary`, like "Zm9vYmFy".

Precision
---------

Time-related datatypes have nanosecond precision.

Precision of decimals and unbounded integers is limited only by the provided
buffer size.  These arbitrary-precision values are opaque, but can be coerced
to simpler datatypes for use.

Dependencies
------------

Exess has no dependencies except for the C standard library, specifically the
functions `ceil`, `ldexp`, `llrint`, `llrintf`, `memcmp`, `memcpy`, `memset`,
`nextafter`, `strcmp`, `strncmp`, `trunc`, and `truncf`.

Documentation
-------------

* [Installation instructions](INSTALL.md)
* [Manual](https://drobilla.gitlab.io/exess/doc/html/)
  * [Single page](https://drobilla.gitlab.io/exess/doc/singlehtml/)
  * [EPub](https://drobilla.gitlab.io/exess/doc/Exess-0.0.1.epub)
  * Installed as `man exess` and related pages

-- 
David Robillard <d@drobilla.net>

[XSD]: https://www.w3.org/TR/xmlschema11-2/
