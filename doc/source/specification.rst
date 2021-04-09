.. _specification:

Specification
=============

Formats:
--------
MorphIO supports ASC (aka Neurolucida), H5 and SWC extensions

Soma formats:
-------------
(For more information, see: `Soma Format <http://neuromorpho.org/SomaFormat.html>`_\ ) MorphIO
recognizes several kinds of soma format. The soma type is determined according to the morphology
type and the number of data points:

Undefined soma SOMA_UNDEFINED
*****************************

MorphIO can read correctly a file without a soma. The latter will be of type "undefined".
Additionnaly, a soma made of 2 points will be of type "undefined" as well. Indeed a 2-points soma
does not make any sense in any of the other soma formats.

Soma single point SOMA_SINGLE_POINT
***********************************

If a soma is made of only one point it will be treated as a "single point soma" regardless of the
morphology file format. The single point soma is represented as a sphere whose coordinate and
radius are given by the soma point.

Morphology file dependent soma type:
************************************

SWC
^^^

`SOMA_NEUROMORPHO_THREE_POINT_CYLINDERS <https://github.com/BlueBrain/MorphIO/blob/a60b52dfe403ef289455ee2221c1b4fce6418978/src/plugin/morphologySWC.cpp#L206>`_
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
SWC files with a soma made of 3 points will have a soma of type 3-points soma. A 3-points soma is
represented as a cylinder along the Y-axis. The first point describe the coordinate of the cylinder
center. Its radius represent the length AND radius of the cylinder. X and Z coordinates of
second and third points must be identical to those of first point. Y coordinate of second (resp.
third) point represents the bottom (resp. top) of the cylinder. Thus, it must have the value Y-R
(resp. Y+R) where Y is the Y coordinate of first point and R the radius.

`SOMA_CYLINDERS <https://github.com/BlueBrain/MorphIO/blob/a60b52dfe403ef289455ee2221c1b4fce6418978/src/plugin/morphologySWC.cpp#L211>`_
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

SWC files with more than 3 points are treated as a stack of consecutive `conical frustums
<http://mathworld.wolfram.com/ConicalFrustum.html>`_. Each point describes coordinates and radius
of a disk. Each frustums is formed by 2 consecutive disks.

H5, ASC
^^^^^^^
ASC and H5 being the text and binary representation of the same format, they obey to the same
specification.

`SOMA_CONTOUR <https://github.com/BlueBrain/MorphIO/blob/a60b52dfe403ef289455ee2221c1b4fce6418978/src/morphology.cpp#L55>`_
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
A H5/ASC file with a soma of 3 points or more will have their soma represented as a soma contour.
In ASC format, soma points are characterized by an S-expression starting with the tag "CellBody".
Even if it is almost never the case in practice, contours are meant to be in the XY plane.

..

   :warning: MorphIO does **not** support ASC files with **multiple** CellBody tags [1]_.



Sub-cellular structures
-----------------------
SWC does not support any sub-cellular structures. H5 and ASC support some, please see the following
for more details.

Spines
******
SWC and H5 do not support spines. ASC files containing files will be read correctly. *However*
spines data are not handled by MorphIO and spine information will be lost when writing to disk.
`Unit test <https://github.com/BlueBrain/MorphIO/blob/a60b52dfe403ef289455ee2221c1b4fce6418978/tests/test_neurolucida.py#L297>`_

Mitochondria
************
SWC and ASC do not support mitochondria. Mitochondria can be read and written to disk using the H5
format. See :ref:`mitochondria-readme`.
for more details about the mitochondria API.
`Unit test <https://github.com/BlueBrain/MorphIO/blob/a60b52dfe403ef289455ee2221c1b4fce6418978/tests/test_immut.py#L43>`_

Custom annotations
******************
Custom annotations are not supported [1]_.

Soma special cases
------------------

No somata
*********
MorphIO supports reading and writing files without a soma. SWC files root sections will have a
parent ID of -1.
`Unit test <https://github.com/BlueBrain/MorphIO/blob/a60b52dfe403ef289455ee2221c1b4fce6418978/tests/test_neurolucida.py#L78>`_\ ,
`Unit test <https://github.com/BlueBrain/MorphIO/blob/a60b52dfe403ef289455ee2221c1b4fce6418978/tests/test_writers.py#L160>`_

Multiple soma
*************
Multiple soma are not supported
`Unit test <https://github.com/BlueBrain/MorphIO/blob/a60b52dfe403ef289455ee2221c1b4fce6418978/tests/test_neurolucida.py#L58>`_\ ,
`Unit test <https://github.com/BlueBrain/MorphIO/blob/d4aeda8d61e824658817f2ecfd8b01fcaca73ab4/tests/test_swc.py#L206>`_

Others
------

Section with only one child section
***********************************
When a section has a single child section (aka unifurcation), the child section will be merged
with its parent when reading or writing the file.
`Unit test <https://github.com/BlueBrain/MorphIO/blob/a60b52dfe403ef289455ee2221c1b4fce6418978/tests/test_neurolucida.py#L251>`_\ ,
`Unit test <https://github.com/BlueBrain/MorphIO/blob/a60b52dfe403ef289455ee2221c1b4fce6418978/tests/test_writers.py#L72>`_

Section ordering
----------------
In MorphIO each section is identified by an ID. By default, the section IDs will correspond to
the order of section appearance while performing a depth-first traversal on every neurites. The
neurite order is the order of appearance in the file. Alternatively, the NRN simulator way of
ordering section can be used by specifying the flag ``morphio::Option::NRN_ID`` when opening
the file. In the NRN simulator, the soma which is considered as a section (contrary to MorphIO)
is placed first and then neurites are sorted according to their type.

The final order is the following:

0. Soma
1. Axon
2. Basal
3. Apical

Format specific specifications
------------------------------
* H5:
  `See the Human Brain Project page <https://developer.humanbrainproject.eu/docs/projects/morphology-documentation/0.0.2/h5v1.html>`_
* ASC:
  :ref:`specification-neurolucida`.
* SWC:
  The file format specification if available on `http://www.neuronland.org <http://www.neuronland.org/NLMorphologyConverter/MorphologyFormats/SWC/Spec.html>`_.
  Here we will discuss, what does MorphIO support and does not.

SWC IDs ordering: there is no special constraint about the IDs as long as the parent ID of each
points is defined. IDs don't need to be consecutive nor sorted, and the soma does not need to be
the first point.

Soma format is determined according to the number of soma points:

* one point -> SOMA_SINGLE_POINT
* two points -> SOMA_UNDEFINED
* three points:

  * layout: one soma point with two children -> SOMA_NEUROMORPHO_THREE_POINT_CYLINDERS
  * else -> SOMA_CYLINDERS

* more than three points -> SOMA_CYLINDERS

.. [1] If this feature seems crucial to you, feel free to create an issue on `MorphIO issue tracker <https://github.com/BlueBrain/MorphIO/issues>`_.
