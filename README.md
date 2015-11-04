[TOC]

# Introduction {#Introduction}

![](doc/BBPLOGO350.jpg)

Welcome to Brion, a C++ library for read and write access to Blue Brain data
structures, including BlueConfig/CircuitConfig, Circuit, CompartmentReport,
Mesh, Morphology, Synapse and Target files.

Brion can be retrieved by cloning the
[source code](https://github.com/BlueBrain/Brion.git).
The [latest API documentation]
(http://bluebrain.github.io/Brion-1.6/index.html) can be found on
[bluebrain.github.io](http://bluebrain.github.io).

To keep track of the changes between releases check the [changelog](@ref Changelog).

# Features {#Features}

Brion provides the following major features:

* Fast and low-overhead read access to:
  * Blue configs (brion::BlueConfig)
  * Circuit description (brion::Circuit)
  * H5 Synapses data (brion::SynapseSummary, brion::Synapse)
  * Target (brion::Target)
  * BBP binary meshes (brion::Mesh)
  * BBP H5 morphologies and SWC morphologies (brion::Morphology)
  * Compartment reports (brion::CompartmentReport)
  * Spike reports (brion::SpikeReport)
* Fast and low-overhead write access to:
  * Compartment reports (brion::CompartmentReport)
  * BBP binary meshes (brion::Mesh)
  * BBP H5 morphologies (brion::Morphology)
* Basic [data types](@ref brion/types.h) to work with the loaded data using
  [vmmlib](http://vmml.github.io/vmmlib),
  [Lunchbox](http://eyescale.github.io/Lunchbox-1.8/index.html),
  [Boost](http://www.boost.org/doc/libs).

# Building {#Building}

Brion is a cross-platform library, designed to run on any modern operating
system, including all Unix variants. Brion uses CMake to create a
platform-specific build environment. The following platforms and build
environments are tested:

* Linux: Ubuntu 14.04 and RHEL 6 (Makefile, i386, x64)

Building from source is as simple as:

    git clone https://github.com/BlueBrain/Brion.git
    mkdir Brion/build
    cd Brion/build
    cmake ..
    make
