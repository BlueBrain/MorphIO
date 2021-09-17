v3.3.1

New Features:
    * Dendritic spine (#321)

Fixes:
    * updated tests
    * use std::make_unique (#333)
    * don't compile c++ tests when creating python package (#332)
    * remove unused AccessMode enum (#331)
    * Imbue enums with numeric operations (#330)
    * allow lowercase neurite names
    * allow whitespace in CellBody neurite

Previously:
- The first neurite point is no longer connected to the soma. It was previously
  the case if the soma was a multiple points soma. This feature has been
  removed as choosing to which point to connect is interpretation dependant and
  should not be the responsability of an IO tool. This feature was already
  discussed [here](https://github.com/BlueBrain/Brion/pull/94#issuecomment-248010437).
  This will impact primarily the surface and volume computations.
