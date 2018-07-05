This page aims at describing what MorphIO supports and does not.

# Formats:
MorphIO supports ASC (aka Neurolucida), H5 and SWC extensions

# Soma formats (See also: http://neuromorpho.org/SomaFormat.html):
MorphIO recognizes several kinds of soma format. The soma type is determined according to the morphology type and the number of data points:
## Undefined soma (SOMA\_UNDEFINED)
MorphIO can read correctly a file without a soma. The latter will be of type "undefined".
Additionnaly, a soma made of 2 points will be of type "undefined" as well. Indeed a 2-points soma does not make any sense in any of the other soma formats.

## Soma single point SOMA\_SINGLE\_POINT
- If a soma is made of only one point it will be treated as a "single point soma" regardless of the morphology file format. 
The single point soma is represented as a sphere whose coordinate and radius are given by the soma point.

## Morphology file dependent soma type:
### SWC
#### SOMA\_NEUROMORPHO\_THREE\_POINT\_CYLINDERS
SWC files with a soma made of 3 points will have a soma of type 3-points soma. A 3-points soma is represented as 
a cylinder along the Y-axis. The first point describe the coordinate of the cylinder center. Its radius represent the length AND radius of the cylinder. X and Z coordinates of
second and third points must be identical to those of first point. Y coordinate of second (resp. third) point represents the bottom (resp. top) of the cylinder. Thus, it must have the value Y-R (resp. Y+R) where Y is the Y coordinate of first point and R the radius.
#### SOMA\_CYLINDERS
SWC files with more than 3 points are treated as a stack of consecutive [[http://mathworld.wolfram.com/ConicalFrustum.html][conical frustums]]. 
Each point describes coordinates and radius of a disk. Each frustums is formed by 2 consecutive disks.

### H5, ASC
ASC and H5 being the text and binary representation of the same format, they obey to the same specification.
#### SOMA\_CONTOUR
A H5/ASC file with a soma of 3 points or more will have their soma represented as a soma contour. In ASC format, soma points are characterized by an S-expression starting with the tag "CellBody". MorphIO does *not* support ASC files with *multiple* CellBody tags. [^footnote]

# Sub-cellular structures
## Spines
ASC files containing files will be read correctly. *However* spines data are not handled by MorphIO and spine information will be lost when writing to disk.

## Mitochondria
Mitochondria can be read and written to disk using the H5 format. See [[https://github.com/BlueBrain/MorphIO/blob/master/README.md#mitochondria][this link]] for more details about the mitochondria API.

## Custom annotations
Custom annotations are not supported. [^footnote]

[^footnote] If this feature seems crucial to you, feel free to create an issue on[[https://github.com/BlueBrain/MorphIO/issues][MorphIO issue tracker]].

