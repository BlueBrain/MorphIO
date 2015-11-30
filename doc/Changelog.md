Changelog {#Changelog}
=========

# git master {#master)

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
