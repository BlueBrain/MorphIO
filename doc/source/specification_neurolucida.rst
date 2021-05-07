Specification Neurolucida
=========================

.. _specification-neurolucida:

Because there is no official Neurolucida specification, this page aims to fill this gap, and provides a subjective
specification of the format.

Soma
----

* Files without a soma are valid.
* Multiple soma is supported via multiple CellBody S-exps, with each of them being the soma contour at a given Z
  altitude. A single CellBody in this definition is called a soma stack. All Z coordinates within a soma stack must
  be the same. Z coordinates among different soma stacks must be different. An example.

  .. code-block:: lisp

      (
        (CellBody) ; <- first soma stack
        (    5.88     0.84    1.0     2.35)
        (    6.05     2.53    1.0     2.35)
        (    6.39     4.38    1.0     2.35)
      )
      (
        (CellBody) ; <- second soma stack
        (    1.85     0.67   2.0     1.35)
        (    0.84     1.52   2.0     1.35)
        (   -4.54     2.36   2.0     1.35)
      )

Duplicate points
----------------

When reading an ASC file, the last point of a section will be added as the first point of the
child sections if not already present. That means these two representations are equivalent:

.. code-block:: lisp

   ( (Dendrite)
     (3 -4 0 2)
     (3 -10 0 2)
     (
       (0 -10 0 2)
       (-3 -10 0 2)
     |
       (6 -10 0 2)
       (9 -10 0 2)
     )
   )


.. code-block:: lisp

   ( (Dendrite)
     (3 -4 0 2)
     (3 -10 0 2)
     (
       (3 -10 0 2) ; <- duplicate
       (0 -10 0 2)
       (-3 -10 0 2)
     |
       (3 -10 0 2) ; <- duplicate
       (6 -10 0 2)
       (9 -10 0 2)
     )
   )

`Unit test <https://github.com/BlueBrain/MorphIO/blob/a60b52dfe403ef289455ee2221c1b4fce6418978/tests/test_neurolucida.py#L162>`_\ ,
`Unit test <https://github.com/BlueBrain/MorphIO/blob/a60b52dfe403ef289455ee2221c1b4fce6418978/tests/test_writers.py#L191>`_

Note: As of today, it is **OK** for a duplicated point to have a different radius than the
original point.

When writing the file the duplicate point is **not** automatically added. However, a warning will
be displayed if the first point of a section differs from the last point of the previous section.

`Unit test <https://github.com/BlueBrain/MorphIO/blob/a60b52dfe403ef289455ee2221c1b4fce6418978/tests/test_mut.py#L125>`_


Single point section
--------------------
A section made of only one point will be treated differently depending on whether the point
is a duplicate of the parent section last point or not.

* If it is a duplicate, the section is discarded. Example:

    .. code-block:: lisp
    
       ((Dendrite)
         (3 -4 0 2)
         (3 -10 0 2)
         (
           (3 -10 0 2)  ; duplicate point
         )
       )

    will become:

    .. code-block:: lisp
    
       ((Dendrite)
         (3 -4 0 2)
         (3 -10 0 2)
       )

    See `here <https://github.com/BlueBrain/MorphIO/blob/5e111f3141f7a1ee72e0260111ce569741d80acb/tests/test_neurolucida.py#L350>`_
    and `here for a more complex case <https://github.com/BlueBrain/MorphIO/blob/5e111f3141f7a1ee72e0260111ce569741d80acb/tests/test_neurolucida.py#L374>`_


* If the point is *not* a duplicate, then the duplicate point is prepended at the beggining of
    the section and the section is considered perfectly valid. See `here <https://github.com/BlueBrain/MorphIO/blob/5e111f3141f7a1ee72e0260111ce569741d80acb/tests/test_neurolucida.py#L253>`_

    .. code-block:: lisp
    
       ((Dendrite)
           (3 -4 0 2)
           (3 -10 0 2)
           (
              (3 -100 100 4)  ; not a duplicate point
           )
       )

    will be equivalent to:

    .. code-block:: lisp

      ((Dendrite)
          (3 -4 0 2)
          (3 -10 0 2)
          (
           (3 -10 0 2)    ; added duplicate
           (3 -100 100 4)
          )
      )

Single child section
--------------------
Section with only one child section will have their child merged with.

.. code-block:: lisp

   ((Dendrite)
    (3 -4 0 2)
    (3 -6 0 2)
    (3 -8 0 2)
    (3 -10 0 2)
    (
      (3 -10 0 2)  ; merged with parent section
      (0 -10 0 2)  ; merged with parent section
      (-3 -15 0 2) ; merged with parent section
      (
        (-5 -5 5 5)
        |
        (-6 -6 6 6)
      )
    )
   )

will be interpreted the same as:

.. code-block:: lisp

   ((Dendrite)
    (3 -4 0 2)
    (3 -6 0 2)
    (3 -8 0 2)
    (3 -10 0 2)
    (0 -10 0 2)
    (-3 -15 0 2)
    (
      (-5 -5 5 5)
      |
      (-6 -6 6 6)
    )
   )

`Unit test <https://github.com/BlueBrain/MorphIO/blob/5e111f3141f7a1ee72e0260111ce569741d80acb/tests/test_neurolucida.py#L285>`_

Empty siblings
**************

File with empty siblings are handled correctly:

.. code-block:: lisp

   ((Dendrite)
    (3 -4 0 2)
    (3 -6 0 2)
    (3 -8 0 2)
    (3 -10 0 2)
    (
      (3 -10 0 2)
      (0 -10 0 2)
      (-3 -10 0 2)
      |       ; <-- empty sibling but still works
     )
    )

will be interpreted the same as:

.. code-block:: lisp

   ((Dendrite)
    (3 -4 0 2)
    (3 -6 0 2)
    (3 -8 0 2)
    (3 -10 0 2)
    (0 -10 0 2)
    )
