.. default-domain:: c
.. highlight:: c

###########
Description
###########

Exess is a simple C library for reading and writing values as strings.

It provides portable locale-independent functions for converting basic number, date, time, and binary datatypes to and from strings.
Conversions are lossless where possible, so, for example,
a ``float`` written to a string will read back as exactly the original value on any system.

The supported datatypes,
defined by the XSD_ specification,
are explicitly compatible with many standards (like XML, RDF, and ISO 8601),
and incidentally or partially compatible with many others (like C and JSON).

The library is straightforward to use and has minimal requirements.
It doesn't depend on an allocator,
the current locale,
or any other shared mutable state,
making it safe to use in almost any context.

*****
Usage
*****

To use exess,
the compiler must be configured to add the versioned include directory to the include path,
and to link with the corresponding library.
The pkg-config_ package ``exess-0`` describes the required options:

.. code-block:: sh

   pkg-config --cflags --libs exess-0

The API can then be used by including ``exess/exess.h``:

.. code-block:: c

   #include <exess/exess.h>

If package support isn't available,
arguments like ``-I/usr/include/exess-0 -lexess-0`` must be added to the compiler command manually.

*********
Datatypes
*********

Most of the datatypes from the XSD_ specification are implemented.
The fully supported primitive datatypes are :ref:`boolean<exess_boolean>`, :ref:`float <exess_float>`, :ref:`double <exess_double>`, :ref:`duration <exess_duration>`, :ref:`dateTime <exess_date_time>`, :ref:`time <exess_time>`, :ref:`date <exess_date>`, :ref:`hexBinary <exess_hex>`, and :ref:`base64Binary <exess_base64>`.
All of the derived datatypes for fixed-size numbers (which correspond to C integer types) are also fully supported: :ref:`long <exess_long>`, :ref:`int <exess_int>`, :ref:`short <exess_short>`, :ref:`byte <exess_byte>`, :ref:`unsignedLong <exess_ulong>`, :ref:`unsignedInt <exess_uint>`, :ref:`unsignedShort <exess_ushort>`, and :ref:`unsignedByte <exess_ubyte>`

The variably-sized :ref:`decimal <exess_decimal>` and ``integer`` are supported to an extent, but values are limited to those that can be stored in a ``double`` or ``int64_t``, respectively.
The library also understands the variably-sized ``integer`` derived datatypes ``nonPositiveInteger``, ``negativeInteger``, ``nonNegativeInteger``, and ``positiveInteger``,
although since these are functionally equivalent to ``integer``,
they don't have their own API functions and are only relevant for some higher-level functionality like value coercion.

**************
Reading Values
**************

Each supported type has a read function that takes a pointer to an output value,
and a string to read.
It reads the value after skipping any leading whitespace,
then returns an :struct:`ExessResult` with a ``status`` code and the ``count`` of characters read.
For example:

.. code-block:: c

   int32_t     v = 0;
   ExessResult r = exess_read_int(&v, "1234");
   if (!r.status) {
     printf("Read %zu bytes as %d\n", r.count, v);
   }

If there was a syntax error,
the status code indicates the specific problem.

Note that read functions may simply stop at unexpected characters.
This allows reading fragments from within some higher-level language,
but leaves the caller responsible for checking that the read ended where expected.
Proper error detection requires checking the returned count as well,
for example:

.. code-block:: c

   bool is_null_terminated_int(const char* string) {
     int32_t     v = 0;
     ExessResult r = exess_read_int(&v, string);

     return !r.status && !string[r.count];
   }

**************
Writing Values
**************

The corresponding write function takes a value to write,
a buffer size in bytes, and a buffer to write to.
It returns an :struct:`ExessResult`,
with a ``status`` code and the ``count`` of characters written,
not including the trailing null byte.

For datatypes with a bounded length,
a constant like :var:`EXESS_MAX_INT_LENGTH` is the maximum length of the canonical representation of any value.
This can be used to allocate buffers statically or on the stack,
for example:

.. code-block:: c

   char s[EXESS_MAX_INT_LENGTH + 1] = {0};

   ExessResult r = exess_write_int(1234, sizeof(s), s);
   if (!r.status) {
     printf("Write error: %s\n", exess_strerror(r.status));
   }

******************
Allocating Strings
******************

Exess never allocates memory,
the calling code is responsible for providing a large enough output buffer.

The `count` returned by write functions can be used to determine the space required for a specific value.
If the write function is called with a null output buffer,
then this count is still returned as if a value were written.
This can be used to precisely allocate memory for the string,
taking care to allocate an extra byte for the null terminator.
For example:

.. code-block:: c

   ExessResult r = exess_write_int(1234, 0, NULL);
   char*       s = (char*)calloc(r.count + 1, 1);

   r = exess_write_int(1234, r.count + 1, s);

Note that for some types,
measuring the output can be about as expensive as actually writing the value.
For example, it requires binary to decimal conversion for floating point numbers.
So, for ``float`` and ``double``,
since the length is bounded and reasonably small,
it's faster to write immediately to a sufficiently large buffer,
then copy the result to the final destination.

**************
Generic Values
**************

The fundamental read and write functions all have similar semantics but different type signatures.
A generic API that works with opaque buffers is also provided,
which can be used to read and write any supported datatype without explicitly handling each case.

Any value can be read with :func:`exess_read_value` and written with :func:`exess_write_value`,
which work similarly to their typed counterparts,
except they take a datatype, size, and pointer to a buffer rather than a value.
:enum:`ExessDatatype` enumerates all of the supported datatypes.

Unbounded Numbers
=================

There are six unbounded number types:
`decimal <https://www.w3.org/TR/xmlschema11-2#decimal>`_,
`integer <https://www.w3.org/TR/xmlschema11-2#integer>`_,
`nonPositiveInteger <https://www.w3.org/TR/xmlschema11-2#nonPositiveInteger>`_,
`negativeInteger <https://www.w3.org/TR/xmlschema11-2#negativeInteger>`_,
`nonNegativeInteger <https://www.w3.org/TR/xmlschema11-2#nonNegativeInteger>`_,
and `positiveInteger <https://www.w3.org/TR/xmlschema11-2#positiveInteger>`_.
Both :func:`exess_read_value` and :func:`exess_write_value` support reading and writing a subset of these types,
but "big" numbers aren't supported.
Values are stored in the largest corresponding native type:
``double``, ``int64_t``, or ``uint64_t``.
If the value doesn't fit,
then :func:`exess_read_value` will return an :enumerator:`EXESS_OUT_OF_RANGE` error.

Canonical Writing
=================

Since values are usually written in canonical form,
strings can be converted to canonical form by first reading, then writing again.
If the parsed value itself isn't required,
then :func:`exess_write_canonical` can be used to do this more efficiently.
For example, this will print ``12``:

.. code-block:: c

   char                s[4] = {0};
   ExessVariableResult r    = {EXESS_SUCCESS, 0, 0};

   r = exess_write_canonical("+12", EXESS_INT, sizeof(s), s);
   if (!r.status) {
     printf("%s\n", s);
   }

This is particularly useful for unbounded datatypes,
since values are transformed one character at a time,
avoiding value conversion, machine limits, and the need for a temporary value buffer.

.. _meson: https://mesonbuild.com/
.. _pkg-config: https://www.freedesktop.org/wiki/Software/pkg-config/
.. _xsd: https://www.w3.org/TR/xmlschema11-2/
