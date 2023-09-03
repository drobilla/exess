#####
Exess
#####

Exess is a simple C library for reading and writing XSD_ datatypes.

.. toctree::

   description

.. only:: man

   ********
   See Also
   ********

   This manual is split into pages that describe a group of related symbols,
   for example,
   :manpage:`exess_int(3)` describes both the read and write functions for ``int32_t``.
   Aliases are also provided for individual functions,
   like :manpage:`exess_read_int(3)` and :manpage:`exess_write_int(3)`.

   Utilities
      :manpage:`exess_symbols(3)`, :manpage:`exess_status(3)`

   Numbers
      :manpage:`exess_decimal(3)`, :manpage:`exess_double(3)`, :manpage:`exess_float(3)`, :manpage:`exess_boolean(3)`, :manpage:`exess_long(3)`, :manpage:`exess_int(3)`, :manpage:`exess_short(3)`, :manpage:`exess_byte(3)`, :manpage:`exess_ulong(3)`, :manpage:`exess_uint(3)`, :manpage:`exess_ushort(3)`, :manpage:`exess_ubyte(3)`

   Date and Time
      :manpage:`exess_duration(3)`, :manpage:`exess_datetime(3)`, :manpage:`exess_timezone(3)`, :manpage:`exess_date(3)`, :manpage:`exess_time(3)`

   Binary
      :manpage:`exess_base64(3)`, :manpage:`exess_hex(3)`

   Datatypes
      :manpage:`exess_datatypes(3)`, :manpage:`exess_canon(3)`, :manpage:`exess_value(3)`

.. only:: not man

   .. toctree::

      api/exess

.. _XSD: https://www.w3.org/TR/xmlschema-2/
