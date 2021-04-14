###########
Using Exess
###########

.. default-domain:: c
.. highlight:: c

The exess C API is declared in ``exess.h``:

.. code-block:: c

   #include <exess/exess.h>

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
     fprintf(stderr, "Read %zu bytes as int %d\n", r.count, value);
   }

If there was a syntax error,
the status code indicates the specific problem.
If a value was read but didn't end at whitespace or the end of the string,
the status :enumerator:`EXESS_EXPECTED_END` is returned.
This indicates that there is trailing garbage in the string,
so the parse may not be complete or correct depending on the context.


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
so the calling code is responsible for providing a large enough buffer for output.
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
it may be better to write immediately to a static buffer,
then copy the result to the final destination.

********
Variants
********

The fundamental read and write functions all have similar semantics,
but different type signatures since they use different value types.
:struct:`ExessVariant` is a tagged union that can hold any supported value,
allowing generic code to work with values of any type.

Any value can be read with :func:`exess_read_variant` and written with :func:`exess_write_variant`,
which work similarly to the fundamental read and write functions,
except the read function takes an additional ``datatype`` parameter.
The expected datatype must be provided,
attempting to infer a datatype from the string content is not supported.

Datatypes
=========

:enum:`ExessDatatype` enumerates all of the supported variant datatypes.
The special value :enumerator:`EXESS_NOTHING` is used as a sentinel for unknown datatypes or other errors.

If you have a datatype URI, then :func:`exess_datatype_from_uri()` can be used
to map it to a datatype.  If the URI is not for a supported datatype, then it will return :enumerator:`EXESS_NOTHING`.

Unbounded Numeric Types
=======================

There are 6 unbounded numeric types:
decimal, integer, nonPositiveInteger, negativeInteger, nonNegativeInteger, and positiveInteger.
:struct:`ExessVariant` supports reading and writing these types,
but stores them in the largest corresponding native type:
``double``, ``int64_t``, or ``uint64_t``.
If the value doesn't fit in this type,
then :func:`exess_read_variant` will return an :enumerator:`EXESS_OUT_OF_RANGE` error.

Writing Canonical Form
======================

Since values are always written in canonical form,
:struct:`ExessVariant` can be used as a generic mechanism to convert any string to canonical form:
simply read a value,
then write it.
If the value itself isn't required,
then :func:`exess_write_canonical` can be used to do this in a single step.
For example, this will print ``123``:

.. code-block:: c

   char buf[4] = {0};

   ExessResult r = exess_write_canonical(" +123", EXESS_INT, sizeof(buf), buf);
   if (!r) {
     printf("%s\n", buf);
   }

Note that it is better to use :func:`exess_write_canonical` if the value isn't required,
since it supports transforming some values outside the range of :struct:`ExessVariant`.
Specifically,
decimal and integer strings will be transformed directly,
avoiding conversion into values and the limits of the machine's numeric types.
