v3.3.3
======

New Features:
* Vasculature section_offsets & section_connectivity (#352)
* n_points method for immutable Morphology and Vasculature (#359)
* is_heterogeneous method for checking downstream/upstream section type homogeneity (#360)

Fixes:
* Fixed documentation links (#350)
* Python version check >=3.7 (#356)
* Updated pybind11 submodule to v2.8.1 (#362)
* Added exception for attempting to write unsupported section types into ascii (#370)
* Wheels for newer python versions
* fix operator<< instances (#364)

v3.3.2
======

Fixes:
* Fixed wheel uploading (#355)

v3.3.1
======

New Features:
* Dendritic spine. Morphology format spec changed from 1.2 to 1.3 (#321)

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
* The first neurite point is no longer connected to the soma. It was previously
  the case if the soma was a multiple points soma. This feature has been
  removed as choosing to which point to connect is interpretation dependant and
  should not be the responsability of an IO tool. This feature was already
  discussed [here](https://github.com/BlueBrain/Brion/pull/94#issuecomment-248010437).
  This will impact primarily the surface and volume computations.
