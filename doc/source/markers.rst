NeuroLucida markers
===================

Specification
*************
A marker is an `s-expression <https://en.wikipedia.org/wiki/S-expression>`__ of the
Neurolucida file that contains additional information about the morphology. For example:

.. code-block:: lisp

  ("pia" ;  String-named marker starts. Such marker can exist only at the top level.
    (Closed)
    (MBFObjectType 5)
    (0 1 2 3)
    (3 4 5 4)
    (6 7 8 5)
    (9 10 11 6)
   ) ;  marker ends

or

.. code-block:: lisp

    ; ...
    ( -277.14  -119.13   -18.02     0.69)
    ( -275.54  -119.99   -16.67     0.69)  ; some section point
    (Cross  ;  common marker starts
      (Color Orange)
      (Name "Marker 3") ; this name will be ignored. 'Cross' will be used as the marker's label.
      ( -271.87  -121.14   -16.27     0.69)  ; 1
      ( -269.34  -122.29   -15.48     0.69)  ; 2
    )  ;  marker ends
    ; ...

The following s-expressions are parsed as markers:

* Any s-exp with a top level string. Like:

    .. code-block:: lisp

      ("pia"
      (Closed)
      (MBFObjectType 5)
      (0 1 2 3)
      (3 4 5 4)
      (6 7 8 5)
      (9 10 11 6)
      )

* An sexp with one of the following regular expression:

    - Dot[0-9]*
    - Plus[0-9]*
    - Cross[0-9]*
    - Splat[0-9]*
    - Flower[0-9]*
    - Circle[0-9]*
    - Flower[0-9]*
    - TriStar[0-9]*
    - OpenStar[0-9]*
    - Asterisk[0-9]*
    - SnowFlake[0-9]*
    - OpenCircle[0-9]*
    - ShadedStar[0-9]*
    - FilledStar[0-9]*
    - TexacoStar[0-9]*
    - MoneyGreen[0-9]*
    - DarkYellow[0-9]*
    - OpenSquare[0-9]*
    - OpenDiamond[0-9]*
    - CircleArrow[0-9]*
    - CircleCross[0-9]*
    - OpenQuadStar[0-9]*
    - DoubleCircle[0-9]*
    - FilledSquare[0-9]*
    - MalteseCross[0-9]*
    - FilledCircle[0-9]*
    - FilledDiamond[0-9]*
    - FilledQuadStar[0-9]*
    - OpenUpTriangle[0-9]*
    - FilledUpTriangle[0-9]*
    - OpenDownTriangle[0-9]*
    - FilledDownTriangle[0-9]*

    Example:

    .. code-block:: lisp

        (FilledCircle
        (Color RGB (64, 0, 128))
        (Name "Marker 11")
        (Set "axons")
        ( -189.59    55.67    28.68     0.12)  ; 1
        )  ;  End of markers

* An 'Incomplete' sexp. Despite it's not the part of the known Neurolucida format, this tag is parsed as marker.

    Example:

    .. code-block:: lisp

        : ...
        ( -269.77  -129.47   -22.57     0.92)  ;
        ( -268.17  -130.62   -24.75     0.92)  ;
        ( -266.79  -131.77   -26.13     0.92)  ; last point before 'Incomplete' happens
         Incomplete
      ) ; end of branch

.. note::
    Markers may have only `(X Y Z)` specified instead of the more common `(X Y Z D)`. In this case,
    diameters are set to 0.

Usage
*****

An instance of morphology has `markers` list attribute that keeps the markers found in the file. A single marker
object within this list is represented as an object with attributes:

- ``section_id``, the id of the section that contains the marker, ``-1`` if there is no section
- ``label``, the text label of the marker
- ``points``, the marker points
- ``diameters``, the marker diameters

.. code-block:: python

    cell = Morphology(os.path.join(_path, 'pia.asc'))
    all_markers = cell.markers
    pia = m.markers[0]

    # fetch the label marker with the `label` attribute
    assert_equal(pia.label, 'pia')
    # fetch the marker's section id
    assert_equal(pia.section_id, -1)

    # fetch the points with the `points` attribute
    assert_array_equal(pia.points,
                         [[0, 1, 2],
                          [3, 4, 5],
                          [6, 7, 8],
                          [9, 10, 11]])

    # fetch the diameters with the `diameters` attribute
    assert_array_equal(pia.diameters, [3, 4, 5, 6])

.. warning::
    Markers with string names are supported only at the top level. Other markers can be presented at any level.
