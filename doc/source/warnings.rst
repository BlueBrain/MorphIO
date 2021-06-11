Warnings
========

MorphIO is trying to be as less rigid as possible in validating of morphologies. That's why most of problems with
morphologies are come as warnings instead of errors. Because of that the output of MorphIO might be verbose.
In order to minimize it you can set the maximum number of warnings, or ignore some of warnings. On the other hand
if you want morphologies to be as valid as possible then you can tell MorphIO to interpret warnings as errors.

Maximum number of warnings
~~~~~~~~~~~~~~~~~~~~~~~~~~
The maximum number of warnings can be set as:

.. code-block:: python

   import morphio

   # Will stop displaying warnings after 100 warnings
   morphio.set_maximum_warnings(100)

   # Will never stop displaying warnings
   morphio.set_maximum_warnings(-1)

   # Warnings won't be displayed
   morphio.set_maximum_warnings(0)

Ignore warnings
~~~~~~~~~~~~~~~

A type of warning can be ignored as shown below. For the complete list of warning types see :class:`morphio.Warning`.

.. code-block:: python

   # ignore single warning
   morphio.set_ignored_warning(morphio.Warning.wrong_root_point, True)
   # stop ignoring it
   morphio.set_ignored_warning(morphio.Warning.wrong_root_point, False)
   # ignore a list of warnings
   morphio.set_ignored_warning([morphio.Warning.no_soma_found,
                                morphio.Warning.zero_diameter], True)
   # stop ignoring them
   morphio.set_ignored_warning([morphio.Warning.no_soma_found,
                                morphio.Warning.zero_diameter], False)

For safety reasons you might want to disable warnings only for construction of morphology, and enable it back
afterwards:

.. code-block:: python

   try:
       morphio.set_ignored_warning(morphio.Warning.wrong_root_point, True)
       m = morphio.Morphology('path/to/morph')
   finally:
       morphio.set_ignored_warning(morphio.Warning.wrong_root_point, False)


Raise warnings
~~~~~~~~~~~~~~
If you want MorphIO to be rigid and fail on any morphology anomaly, you can switch to raise warnings as errors.
This way they also can be caught programmatically.

.. code-block:: python

   morphio.set_raise_warnings(True)

For safety reasons you might want to raise warnings only for construction of morphology, and disable it back
afterwards:

.. code-block:: python

   try:
       morphio.set_raise_warnings(True)
       m = morphio.Morphology('path/to/morph')
   finally:
       morphio.set_raise_warnings(False)

If you care only about some types of warnings then you need to ignore others:

.. code-block:: python

   try:
       morphio.set_raise_warnings(True)
       # warnings you are not interested in
       morphio.set_ignored_warning([morphio.Warning.wrong_root_point, ...], True)
       m = morphio.Morphology('path/to/morph')
   finally:
       morphio.set_ignored_warning([morphio.Warning.wrong_root_point, ...], False)
       morphio.set_raise_warnings(False)
