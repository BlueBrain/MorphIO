Compartment report data model {#data_compartments}
============

## Prerequisites

A _neuron_ is a single nerve cell simulated, and connected with thousands of
other neurons. It is constructed from a morphological skeleton aka _morphology_.
This skeleton has a tree-like structure originating at the soma, the cell body
of each neuron. Each neuron has a unique _GID_ (global identifier). Every neuron
represents an instance of a certain morphology in 3D space which is link in the
MVD circuit description.

The branches of the tree are formed by _sections_ which are subdivided into
smaller pieces. They start at the soma and eventually branch into multiple
sections in a tree-like structure.

During the reconstruction from real neurons, each section is formed by a number
of _segments_. Each segment has two endpoints in 3D space, as well as a radius
for each endpoint. These segments consequently represent tubelets in 3D space.

For the simulation, each section is subdivided in m equally long _compartments_.
For each compartment, various observables are simulated and may be reported in a
CompartmentReport.

## Meta data

* List of neuron GIDs
* Start time in ms
* End time in ms
* Dt in ms
* Data unit as string, e.g. mV, pA
* Morphological skeletons describing the cell tree structure in sections
  and segments

## Compartment information

* For each neuron, sorted ascending by GID
* Number of compartments of each section per neuron, sections order defined by
  morphology
* All sections of a neuron are always described, i.e. unreported sections have
  zero compartments

Compartments and their reported values are linearly interpolated along
all segments of a section, i.e. all compartments have the same length
within a particular section, and when computing the value of a segment
linear interpolation of all compartment contributions in this segment is
used.

## Report information

* Reportable value in data unit per compartment
* Sorted ascending by timestamp, discretized in (_endtime - starttime_) / _dt_
  frames
* Within a timestamp, sorted ascending by GID
* Within a neuron, sorted by section order defined by morphology

Typically, implementations provide a mapping information to correlate GID and
section to number of compartments and their reported value.
