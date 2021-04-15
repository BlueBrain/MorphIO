Warnings
========

Maximum number of warnings
~~~~~~~~~~~~~~~~~~~~~~~~~~
On can control the maximum number of warnings using the command:

.. code-block:: python

   # Will stop displaying warnings after 100 warnings
   morphio.set_maximum_warnings(100)

   # Will never stop displaying warnings
   morphio.set_maximum_warnings(-1)

   # Warnings won't be displayed
   morphio.set_maximum_warnings(0)

Raise warnings
~~~~~~~~~~~~~~
Because MorphIO warning is just output to stdout/stderr, one can switch to raise warnings as errors, so they can be
caught programmatically:

.. code-block:: python

   morphio.set_raise_warnings(True)
