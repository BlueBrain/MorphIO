This page aims at describing what MorphIO supports and does not.

# Formats:
MorphIO supports ASC (aka Neurolucida), H5 and SWC extensions

# Soma formats (See also: http://neuromorpho.org/SomaFormat.html):
MorphIO recognizes several kinds of soma format. The soma type is determined according to the morphology type and the number of data points:
## Undefined soma SOMA\_UNDEFINED
MorphIO can read correctly a file without a soma. The latter will be of type "undefined".
Additionnaly, a soma made of 2 points will be of type "undefined" as well. Indeed a 2-points soma does not make any sense in any of the other soma formats.

## Soma single point SOMA\_SINGLE\_POINT
If a soma is made of only one point it will be treated as a "single point soma" regardless of the morphology file format.
The single point soma is represented as a sphere whose coordinate and radius are given by the soma point.

## Morphology file dependent soma type:
### SWC

#### [SOMA\_NEUROMORPHO\_THREE\_POINT\_CYLINDERS](https://github.com/BlueBrain/MorphIO/blob/a60b52dfe403ef289455ee2221c1b4fce6418978/src/plugin/morphologySWC.cpp#L206)
SWC files with a soma made of 3 points will have a soma of type 3-points soma. A 3-points soma is represented as
a cylinder along the Y-axis. The first point describe the coordinate of the cylinder center. Its radius represent the length AND radius of the cylinder. X and Z coordinates of
second and third points must be identical to those of first point. Y coordinate of second (resp. third) point represents the bottom (resp. top) of the cylinder. Thus, it must have the value Y-R (resp. Y+R) where Y is the Y coordinate of first point and R the radius.
#### [SOMA\_CYLINDERS](https://github.com/BlueBrain/MorphIO/blob/a60b52dfe403ef289455ee2221c1b4fce6418978/src/plugin/morphologySWC.cpp#L211)
SWC files with more than 3 points are treated as a stack of consecutive [conical frustums](http://mathworld.wolfram.com/ConicalFrustum.html).
Each point describes coordinates and radius of a disk. Each frustums is formed by 2 consecutive disks.

### H5, ASC
ASC and H5 being the text and binary representation of the same format, they obey to the same specification.

#### [SOMA\_CONTOUR](https://github.com/BlueBrain/MorphIO/blob/a60b52dfe403ef289455ee2221c1b4fce6418978/src/morphology.cpp#L55)
A H5/ASC file with a soma of 3 points or more will have their soma represented as a soma contour. In ASC format, soma points are characterized by an S-expression starting with the tag "CellBody". MorphIO does *not* support ASC files with *multiple* CellBody tags. [^footnote]

# Sub-cellular structures
SWC does not support any sub-cellular structures. H5 and ASC support some, please see the following for more details.

## Spines
SWC and H5 do not support spines.
ASC files containing files will be read correctly.
*However* spines data are not handled by MorphIO and spine information will be lost when writing to disk.
[Unit test](https://github.com/BlueBrain/MorphIO/blob/a60b52dfe403ef289455ee2221c1b4fce6418978/tests/test_neurolucida.py#L297)

## Mitochondria
SWC and ASC do not support mitochondria.
Mitochondria can be read and written to disk using the H5 format. See the [README.md](https://github.com/BlueBrain/MorphIO/blob/master/README.md#mitochondria) for more details about the mitochondria API.
[Unit test](https://github.com/BlueBrain/MorphIO/blob/a60b52dfe403ef289455ee2221c1b4fce6418978/tests/test_immut.py#L43)

## Custom annotations
Custom annotations are not supported. [^footnote]

# Soma special cases
## No somata
MorphIO supports reading and writing files without a soma. SWC files root sections will have a parent ID of -1.
[Unit test](https://github.com/BlueBrain/MorphIO/blob/a60b52dfe403ef289455ee2221c1b4fce6418978/tests/test_neurolucida.py#L78), [Unit test](https://github.com/BlueBrain/MorphIO/blob/a60b52dfe403ef289455ee2221c1b4fce6418978/tests/test_writers.py#L160)

## Multiple soma
Multiple soma are not supported
[Unit test](https://github.com/BlueBrain/MorphIO/blob/a60b52dfe403ef289455ee2221c1b4fce6418978/tests/test_neurolucida.py#L58), [Unit test](https://github.com/BlueBrain/MorphIO/blob/d4aeda8d61e824658817f2ecfd8b01fcaca73ab4/tests/test_swc.py#L206)

# Others
## Section with only one child section
When a section has a single child section (aka unifurcation), the child section will be merged with its parent when reading or writing the file.

[Unit test](https://github.com/BlueBrain/MorphIO/blob/a60b52dfe403ef289455ee2221c1b4fce6418978/tests/test_neurolucida.py#L251), [Unit test](https://github.com/BlueBrain/MorphIO/blob/a60b52dfe403ef289455ee2221c1b4fce6418978/tests/test_writers.py#L72)

## Duplicate points
When reading an ASC file, the last point of a section will be added as the first point of the
child sections if not already present. That means these two representations are equivalent:

```lisp
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
```

```lisp
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
```

[Unit test](https://github.com/BlueBrain/MorphIO/blob/a60b52dfe403ef289455ee2221c1b4fce6418978/tests/test_neurolucida.py#L162), [Unit test](https://github.com/BlueBrain/MorphIO/blob/a60b52dfe403ef289455ee2221c1b4fce6418978/tests/test_writers.py#L191)

Note: As of today, it is **OK** for a duplicated point to have a different radius than the original point.


When writing the file the duplicate point is **not** automatically added. However, a warning will be displayed if the first point of a section differs from the last point of the previous section.

[Unit test](https://github.com/BlueBrain/MorphIO/blob/a60b52dfe403ef289455ee2221c1b4fce6418978/tests/test_mut.py#L125)


[^footnote] If this feature seems crucial to you, feel free to create an issue on [MorphIO issue tracker](https://github.com/BlueBrain/MorphIO/issues).
