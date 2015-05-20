ChangeLog  {#changelog}
=========

[TOC]

# Brion 1.5.0 {#changelog_1_5_0}

## New Features {#NewFeatures_1_5_0}

* Morphology class implementation changed to support the Lunchbox plugin
  mechanism.
* New morphology plugin to read SWC morphologies.

## Enhancements {#Enhancements_1_5_0}

* No enhancements.

## Optimizations {#Optimizations_1_5_0}

* No optimizations.

## Documentation {#Documentation_1_5_0}

* No documentation updates.

## Bug Fixes {#Fixes_1_5_0}

* No bug fixes.

## Known Bugs {#Bugs_1_5_0}

The following bugs were known at release time. Please file a
[Bug Report](https://github.com/BlueBrain/Brion/issues) if you find any other
issue with this release.

* No known bugs.

# Brion 1.4.0 {#changelog_1_4_0}

## New Features {#NewFeatures_1_4_0}

* Old report readers have been adapted to be static plugins using
  lunchbox::IOPluginFactory.
* Added a new spike report reader plugin to handle NEST spike reports.
  The plugin can parse multiple report file using shell wildcards (*, ?) in
  the filepath provided.
* A new constructor for brion::CompartmentReport accepting a URI and the desired
  access mode deprecates the old read and write constructors. This
  constructor is required for the new plugin mechanism.
* Added a new compartment report based on key-value stores supported by
  Lunchbox::PersistentMap.
* brion::SpikeReport API extended to support stream-based reports. A reference
  implementation of a stream-based spike report plugin is also provided.

## Enhancements {#Enhancements_1_4_0}

* No enhancements.

## Optimizations {#Optimizations_1_4_0}

* Spike loading has been optimized (in a 12-core GPU the expected
speed-up is above 4x for large files).

## Documentation {#Documentation_1_4_0}

* No documentation updates.

## Bug Fixes {#Fixes_1_4_0}

Brion 1.4 includes various bugfixes over the last release, including the
following:

* [295:](https://bbpteam.epfl.ch/project/issues/browse/BBPSDK-295) BBPSDK 21
  cannot load v2 morphologies.

## Known Bugs {#Bugs_1_4_0}

The following bugs were known at release time. Please file a
[Bug Report](https://github.com/BlueBrain/Brion/issues) if you find
any other issue with this release.

* No known bugs.
