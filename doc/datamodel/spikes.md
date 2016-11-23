Spike report data model {#data_spikes}
============

## Prerequisites

A _neuron_ is a single nerve cell simulated, and connected with thousands of
other neurons. Each neuron has a unique _GID_ (global identifier).

Spikes are reported continuously, contrary to frame-based compartment reports.

## Meta data

* List of neuron GIDs
* Start time in ms
* End time in ms

## Report information

* Spikes: pair of timestamp in ms and GID
