v3.3.3
======

New Features
    * Vasculature section_offsets & section_connectivity (#352)
    * n_points method for immutable Morphology and Vasculature (#359)

Fixes:
    * Fixed documentation links (#350)
    * Python version check >=3.7 (#356)
    * Updated pybind11 submodule to v2.8.1 (#362)

v3.3.2
======

Fixes:
    * Fixed wheel uploading (#355)

v3.3.1
======

New Features:
    * Dendritic spine (#321)

Fixes:
    * Imbue enums with numeric operations (#330)
    * Remove unused AccessMode enum (#331)
    * Don't compile c++ tests when creating python package (#332)
    * Use std::make_unique (#333)
    * Updated tests (#340)
    * Allow lowercase neurite names
    * Allow whitespace in CellBody neurite
    * Excluded musslinux wheels from building (#354)

Previously:
- The first neurite point is no longer connected to the soma. It was previously
  the case if the soma was a multiple points soma. This feature has been
  removed as choosing to which point to connect is interpretation dependant and
  should not be the responsability of an IO tool. This feature was already
  discussed [here](https://github.com/BlueBrain/Brion/pull/94#issuecomment-248010437).
  This will impact primarily the surface and volume computations.
