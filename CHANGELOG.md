v3.3.7
======
* Change license from LGPL-3.0 to Apache-2.0 #467
* Switch to HDF5 1.14 for all wheels
* Add wheel building for python 3.12
* Add wheel building for ARM64 Apple Silicon
* Better write for soma types #411

v3.3.6
======
Improvements:
* don't allow negative IDs in SWC, except for the special parent == -1, for the first sample (#466)
* Bump HighFive to v2.7.1 (#449)
* Increase allowed section type to 19 (#462)
* Have warnings for incompatible soma-types (#458):
    * SWC files should have soma that are either a point, or a set of stacked cylinders.
    * H5 & ASC consider the soma to be a contour
    * allow assignment of soma type for mutable soma
    * add errors for soma access / write if wrong size
     - ERROR_SOMA_INVALID_SINGLE_POINT
     - ERROR_SOMA_INVALID_THREE_POINT_CYLINDER
     - ERROR_SOMA_INVALID_CONTOUR
* increased test coverage and cleanup

v3.3.5
======
New Features:
* Abstraction for morphology collection. (#444)

Improvements:
* Don't ignore choice of `MORPHIO_ENABLE_COVERAGE`. (#452)
* enable windows py311, since new h5py release has wheels (#441)
* Update clang-format CI to 22.04. (#445)
* Make neurolucida state machine thread-safe (#440)
* Improve doc (#439)

v3.3.4
======

New Features:
* Add operator-> for iterators, for better ergonomics (#408)
* Allow SWC and ASC morphologies to be built from strings (#407)

Fixes:
* Replace assert by exception (#436)
* Render README as RST (#434)
* Fallback to using python 3.10 for ci tests (#430)
* Fix sscanf clammping of allowed inputs (#420)
* Fix section order after deleting section in mut morphology (#412)
* Enable test mistakenly disabled (#418)
* Freeze wheel build os versions (#391)
* Fix wrong section order when section is deleted (#412)
* Add 3.11 wheels Linux; macOS/Windows ones require h5py to release an updated wheel for python 3.11

Improvements:
* Remove travis configuration and badge (#428)
* Update links to morphology documentation (#424)
* Update pybind11 to v2.10.0 release (#425)
* Cleanup tests (#421)
* Cleanup mutable morphology and soma (#413, #414, #415)
* Cleanup ascii reader (#405)
* Use Catch2 builtins to approximate comparisons (#394)
* Update lexertl14 to latest commit cd5a1f1 (#397)
* Update HighFive to v2.4.1 (#406)
* Update gsl-lite submodule to v0.40.0 (#395)
* Use non-exceptional methods wherever possible (#389)
* Rebuild morphologies only when modifiers are passed (#392)


v3.3.3
======

New Features:
* Vasculature `section_offsets` & `section_connectivity` (#352)
* `n_points method` for immutable Morphology and Vasculature (#359)
* `is_heterogeneous` method for checking downstream/upstream section type homogeneity (#360)

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
* Use `std::make_unique` (#333)
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
