###########
Using Exess
###########

.. default-domain:: cpp
.. highlight:: cpp
.. namespace:: exess

The exess C++ API is declared in ``exess.hpp``:

.. code-block:: cpp

   #include <exess/exess.hpp>

**************
Reading Values
**************

Each supported type has a read function that takes a pointer to an output value,
and a string to read.
It reads the value after skipping any leading whitespace,
then returns an :struct:`ExessResult` with a ``status`` code and the ``count`` of characters read.
For example:

.. code-block:: cpp

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
the `constexpr` function template :func:`max_length` returns the maximum length of the canonical representation of any value.
This can be used to allocate buffers statically or on the stack,
for example:

.. code-block:: cpp

   char buf[exess::max_length<int>() + 1] = {0};

   exess::Result r = exess::write(1234, sizeof(buf), buf);
   if (r.status != exess::Status::success) {
     std::cerr << "Write error: " << exess::strerror(r.status) << "\n";
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

.. code-block:: cpp

   exess::Result r   = exess::write(1234, 0, NULL);
   char*         str = (char*)calloc(r.count + 1, 1);

   r = exess_write_int(1234, r.count + 1, buf);

Note that for some types,
this operation can be about as expensive as actually writing the value.
For example, it requires binary to decimal conversion for floating point numbers.
For ``float`` and ``double``,
since the length is bounded and relatively small,
it may be better to write immediately to a static buffer,
then copy the result to the final destination.
