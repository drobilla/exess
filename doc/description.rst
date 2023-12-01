###########
Description
###########

.. default-domain:: c
.. highlight:: c

Exess provides portable, locale-independent, and standards-backed functions for converting common numeric and temporal datatypes to and from strings.
Conversions are lossless wherever possible so,
for example,
a ``float`` written to a string will read back as exactly the original value on any system.

*****
Usage
*****

To use exess,
the compiler must be configured to add the versioned include directory to the include path,
and to link with the corresponding library.
Depending on the package ``exess-0`` achieves this in most build systems,
or the required flags can be fetched manually with pkg-config_ or a compatible tool:

.. code-block:: sh

   pkg-config --cflags --libs exess-0

The API can then be used by including ``exess/exess.h``:

.. code-block:: c

   #include <exess/exess.h>

Exess can also be used as a meson_ subproject.
If neither meson nor pkg-config are available,
things will need to be configured manually,
for example by passing compiler options like ``-I/usr/include/exess-0 -lexess-0``.

**************
Reading Values
**************

Each supported type has a read function that takes a pointer to an output value,
and a string to read.
It reads the value after skipping any leading whitespace,
then returns an :struct:`ExessResult` with a ``status`` code and the ``count`` of characters read.
For example:

.. code-block:: c

   int32_t     value = 0;
   ExessResult r     = exess_read_int(&value, "1234");
   if (!r.status) {
     printf("Read %zu bytes as %d\n", r.count, value);
   }

If there was a syntax error,
the status code indicates the specific problem.
If a value was read but didn't end at whitespace or the end of the string,
the status :enumerator:`EXESS_EXPECTED_END` is returned.
This indicates that there is trailing garbage in the string,
so the parse may be incomplete or incorrect depending on the context.

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

   char buf[EXESS_MAX_INT_LENGTH + 1] = {0};

   ExessResult r = exess_write_int(1234, sizeof(buf), buf);
   if (!r.status) {
     printf("Write error: %s\n", exess_strerror(r.status));
   }

******************
Allocating Strings
******************

Exess doesn't do any allocation itself,
so the calling code is responsible for providing a large enough output buffer.
The `count` returned by write functions can be used to determine the space required for a specific value.
If the write function is called with a null output buffer,
then this count is still returned as if a value were written.
This can be used to precisely allocate memory for the string,
taking care to allocate an extra byte for the null terminator.
For example:

.. code-block:: c

   ExessResult r   = exess_write_int(1234, 0, NULL);
   char*       str = (char*)calloc(r.count + 1, 1);

   r = exess_write_int(1234, r.count + 1, buf);

Note that for some types,
this operation can be about as expensive as actually writing the value.
For example, it requires binary to decimal conversion for floating point numbers.
For ``float`` and ``double``,
since the length is bounded and relatively small,
it's likely faster to write immediately to a sufficiently large buffer,
then copy the result to the final destination.

**************
Generic Values
**************

The fundamental read and write functions all have similar semantics,
but different type signatures since they use different value types.
An alternative API that works with opaque buffers is also provided,
which allows for reading and writing any supported datatype without explicitly handling each case.

Any value can be read with :func:`exess_read_value` and written with :func:`exess_write_value`,
which work similarly to their typed counterparts,
except they take a datatype, size, and pointer to a buffer rather than a value.
:enum:`ExessDatatype` enumerates all of the supported datatypes.

Unbounded Numeric Types
=======================

There are 6 unbounded numeric types:
decimal, integer, nonPositiveInteger, negativeInteger, nonNegativeInteger, and positiveInteger.
The generic value functions support reading and writing these types,
but store them in the largest corresponding native type:
``double``, ``int64_t``, or ``uint64_t``.
If the value doesn't fit in this type,
then :func:`exess_read_value` will return an :enumerator:`EXESS_OUT_OF_RANGE` error.

Writing Canonical Form
======================

Since values are always written in canonical form,
the generic value functions can be used to convert any string to canonical form:
simply read the value,
then write it again.
If the value itself isn't required,
then :func:`exess_write_canonical` can be used to do this more efficiently.
For example, this will print ``12``:

.. code-block:: c

   char        buf[4] = {0};
   ExessResult r      = exess_write_canonical(
     "+12", EXESS_INT, sizeof(buf), buf);

   if (!r.status) {
     printf("%s\n", buf);
   }

Note that when the value isn't needed,
it's better to use :func:`exess_write_canonical` since it supports transforming large values directly.
Specifically,
unbounded numbers and binary values are transformed a character at a time,
avoiding value conversion,
the limits of the machine's numeric types,
and the need for buffer space to store the value.

.. _meson: https://mesonbuild.com/
.. _pkg-config: https://www.freedesktop.org/wiki/Software/pkg-config/
