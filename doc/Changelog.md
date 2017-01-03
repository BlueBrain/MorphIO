Changelog {#Changelog}
=========

# git master

* [121](https://github.com/BlueBrain/Brion/pull/121):
  Performance optimizations for map and binary compartment reports, fix
  sub target loading of map compartment reports
* [119](https://github.com/BlueBrain/Brion/pull/119):
  Optimized brain::Circuit::loadMorphologies for slow stat'ing
  filesystems (GPFS)

# Release 1.9.0 (09-Dec-2016)

* [117](https://github.com/BlueBrain/Brion/pull/117):
  Changes and fixes in Brain Python module:
  - Functions that take gids now preserve the iteration order of the input in
    the output arrays/lists.
  - Replaced Python sets with numpy arrays in functions returning GID sets.
  - Circuit::getXYZNames functions renamed to Circuit::getXYZTypeNames.
* [113](https://github.com/BlueBrain/Brion/pull/113):
  Support for old circuits containing only synapse center positions.
* [110](https://github.com/BlueBrain/Brion/pull/110):
  Break PersistentMap out into keyv::Map
* [107](https://github.com/BlueBrain/Brion/pull/107):
  Added Sphinx generated documentation of the brain python module.
* [102](https://github.com/BlueBrain/Brion/pull/102):
  Use PersistentMap for caching synapse positions loaded from brain::Circuit
* [94](https://github.com/BlueBrain/Brion/pull/94):
  Fixed SWC morphology parser for morphologies with soma contour. The parser was
  creating invalid soma sections when the first order sections where connected
  to arbitrary soma sample points.
* [89](https://github.com/BlueBrain/Brion/pull/89):
  Python wrapping of brain classes.
* [88](https://github.com/BlueBrain/Brion/pull/88):
  - Brain namespace enums made strongly typed.
  - Fix for brain::Section::getSamples for sections where the first point
    appears repeated.
* [85](https://github.com/BlueBrain/Brion/pull/85):
  Implement brain synapses specification
* [84](https://github.com/BlueBrain/Brion/pull/84):
  Brain synapses specification
* [83](https://github.com/BlueBrain/Brion/pull/83):
  Add brain::Circuit::getRandomGIDs()
* [82](https://github.com/BlueBrain/Brion/pull/82):
  Improve performance when reading synapse attributes from non-merged files
* [81](https://github.com/BlueBrain/Brion/pull/81):
  Fix GID out-of-bounds handling for MVD3 in brain::Circuit
* [79](https://github.com/BlueBrain/Brion/pull/79):
  Use PersistentMap for cache in brain::Circuit::loadMorphologies(); add thread
  safety w/ synapses cache

# Release 1.8.0 (30-Jun-2016)

* [77](https://github.com/BlueBrain/Brion/pull/77):
  Add brain::Circuit::getMorphologyTypes() and brain::Circuit::getElectrophysiologyTypes()
* [75](https://github.com/BlueBrain/Brion/pull/75):
  Implement morphology version 1.1 specification
* [74](https://github.com/BlueBrain/Brion/pull/74):
  Remove deprecated enums and functions:
  - `CompartmentReport( const std::string&, const GIDSet& )` and
    `CompartmentReport( const std::string&,
    const brion::CompartmentReportFormat, const bool )` constructors; use
    brion::CompartmentReport::CompartmentReport( const brion::URI&, int, const brion::GIDSet& )
    instead
  - `enum CompartmentReportFormat`; use brion::BlueConfig::getReportSource()
    instead
* [71](https://github.com/BlueBrain/Brion/pull/71):
  Transparent caching of Synapse loading. See
  [Lunchbox#263](https://github.com/Eyescale/Lunchbox/pull/263) for
  configuration.
* [69](https://github.com/BlueBrain/Brion/pull/69):
  Speedup brain::Circuit::getGIDs(), add brion::BlueConfig::getTargetSources()
* [63](https://github.com/BlueBrain/Brion/pull/63):
  Moved old BBPSDK/Monsteer spike report to Brain (pending refactoring)

# Release 1.7.0 (24-Mar-2016)

* [56](https://github.com/BlueBrain/Brion/pull/56):
  Improved target parser
* [55](https://github.com/BlueBrain/Brion/pull/55):
  Add basic provenance metadata for written HDF5 compartment reports
* [49](https://github.com/BlueBrain/Brion/pull/49):
  Added the method brain::Circuit::getRotations
* [46](https://github.com/BlueBrain/Brion/pull/46):
  Fixed a bug in enum to string conversions affecting morphologyConverter.
* [45](https://github.com/BlueBrain/Brion/pull/45):
  Made targets optional.
  - brain::Circuit::getGIDs() returns all GIDs handled by the circuit.
  - brain::Circuit::getGIDs( target ) returns GIDS for a specified target.
  - brain::Circuit::getNumNeurons() returns the total number of neurons in the
    circuit.
* [43](https://github.com/BlueBrain/Brion/pull/43):
  Add MVD3 support to brain::circuit
* [39](https://github.com/BlueBrain/Brion/pull/39):
  Add compartment report converter tool
* [30](https://github.com/BlueBrain/Brion/pull/30),
  [35](https://github.com/BlueBrain/Brion/pull/35):
  Added a new library, Brain, to provide higher level functions and classes.
  The library provides:
  - A Circuit class to get basic information from cells and targets and load
    morphologies at circuit level.
  - A brain::cell namspace with a Morphology class plus other related classes
    to access morphological data about neurons.
* [38](https://github.com/BlueBrain/Brion/pull/38):
  Fix crash while reading more than `ulimit -Sn` (1024 default) morphologies
* [37](https://github.com/BlueBrain/Brion/pull/37):
  Added support for synapse nrn_extra.h5 files.
* [31](https://github.com/BlueBrain/Brion/pull/31):
  Fix crash while reading more than `ulimit -Sn` (1024 default) NEST gdf files
* [29](https://github.com/BlueBrain/Brion/pull/29):
  New member functions in brion::BlueConfig to provide a semantic API.
* [28](https://github.com/BlueBrain/Brion/pull/28):
  SpikeReport continues parsing files that have broken lines

# Release 1.6.0 (9-Nov-2015)

* [24](https://github.com/BlueBrain/Brion/pull/24):
  Add getURI() method in SpikeReport.
* [22](https://github.com/BlueBrain/Brion/pull/22):
  spikeConverter can process spikes to and from stream-type SpikeReport plugins.
* [22](https://github.com/BlueBrain/Brion/pull/22):
  SpikeReport DSO plugins in the LD_LIBRARY_PATH are loaded automatically.
* [12](https://github.com/BlueBrain/Brion/pull/12):
  Extended brion::Synapse to also support non-merged synapse files
* [12](https://github.com/BlueBrain/Brion/pull/12):
  Add brion::Target::parse() to resolve a given target name
* [9](https://github.com/BlueBrain/Brion/issues/9):
  Extend SWC parser to support fork and end points and undefined section points.
  The Brion::SectionType enum has not been extended to include end and fork
  points, these types are translated into the most reasonable one based on the
  point ancestors.

# Release 1.5.0 (7-Jul-2015)

* Add RESTING_VOLTAGE constant.
* Add support for binary spike reports.
* Add support for spike writer plugin.
* Add async IO support in brion::CompartmentReportMap.
* brion::CompartmentReportBinary::updateMapping() more robust.
* brion::SpikeReport API extended to support stream-based reports. A reference
  implementation of a stream-based spike report plugin is also provided.
* Ensure thread-safety with non-threadsafe HDF5 library.
* Morphology class implementation changed to support the Lunchbox plugin
  mechanism.
* New morphology plugin to read SWC morphologies.
* [#3](https://github.com/BlueBrain/Brion/pull/3):
  New morphology converter tool.
* New null compartment report for benchmarks.
* New spike converter tool.
* Optimized function Circuit::get for large circuits (cut down the loading time
  of the 3.1 M neuron circuit by 6.5 seconds in a Core i7-4930K @ 3.40GHz).
* Replace use of iostreams by lunchbox::MemoryMap.
* Several bugfixes and cleanups.

# Release 1.4.0 (8-Oct-2014)

* Old report readers have been adapted to be static plugins using
  lunchbox::IOPluginFactory.
* New spike report reader plugin for NEST spike reports. The plugin can
  parse multiple report file using shell wildcards (*, ?) in the
  filepath provided.
* New constructor for brion::CompartmentReport accepting an URI and the desired
  access mode deprecates the old read and write constructors.
* New compartment report based on key-value stores supported by
  Lunchbox::PersistentMap.
* Spike loading has been optimized (in a 12-core GPU the expected
  speed-up is above 4x for large files).
