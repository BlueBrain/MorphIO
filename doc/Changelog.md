Changelog {#Changelog}
=========

## git master {master)

* [#12](https://github.com/BlueBrain/Brion/pull/12):
  Add brion::Target::parse() to resolve a given target name
* [#12](https://github.com/BlueBrain/Brion/pull/12):
  Extended brion::Synapse to also support non-merged synapse files

## Release 1.5.0 (2015-07-07) {changelog_1_5_0}

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

## Release 1.4.0 (2014-10-08) {changelog_1_4_0}

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

## Known Bugs {Bugs}

The following bugs were known at release time. Please file a
[Bug Report](https://github.com/BlueBrain/Brion/issues) if you find
any other issue with this release.

* No known bugs.
