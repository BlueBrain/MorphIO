ChangeLog  {#changelog}
=========

[TOC]

# Brion 1.5.0 {#changelog_1_5_0}

* New morphology plugin to read SWC morphologies.
* New morphology converter tool
* Morphology class implementation changed to support the Lunchbox plugin
  mechanism.

# Brion 1.4.0 {#changelog_1_4_0}

* Old report readers have been adapted to be static plugins using
  lunchbox::IOPluginFactory.
* New spike report reader plugin for NEST spike reports. The plugin can
  parse multiple report file using shell wildcards (*, ?) in the
  filepath provided.
* New constructor for brion::CompartmentReport accepting an URI and the desired
  access mode deprecates the old read and write constructors.
* New compartment report based on key-value stores supported by
  Lunchbox::PersistentMap.
* brion::SpikeReport API extended to support stream-based reports. A reference
  implementation of a stream-based spike report plugin is also provided.
* Spike loading has been optimized (in a 12-core GPU the expected
  speed-up is above 4x for large files).

## Known Bugs {#Bugs}

The following bugs were known at release time. Please file a
[Bug Report](https://github.com/BlueBrain/Brion/issues) if you find
any other issue with this release.

* No known bugs.
