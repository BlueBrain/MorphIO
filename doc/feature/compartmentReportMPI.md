Compartment report writing from MPI applications {#compartment_report_mpi}
============

This feature is a modification of the CompartmentReport to allow MPI
programs to write reports. The first use case is to write into a
key-value store from CoreNeuron.

## Requirements

* Efficient parallel write from an MPI application, for now only for the
  map-based reports (no duplicate writes, decoupled MPI processes, i.e.,
  no barrier needed)
* Parallelization across the data domain for full neurons, i.e., no
  parallel writing of a frame for a single GID

## API

New parameter 'gids' to CompartmentReport::writeHeader (see
Implementation section).

## Examples

    Rank #0:
        CompartmentReport report;
        report.writeHeader( start, end, step, unit, mv, ms, gids [new] );

    All ranks:
        CompartmentReport report;
        for each local compartment
            report.writeCompartments( gid, counts );
        for each time step
            for each local compartment
                report.writeFrame( gid, voltages, time );


## Implementation

Add gids explicitely to writeHeader, so all the header information can
be written directly from rank #0. Each rank is then responsible of
writing the mapping for its gids and all time steps. The implementation
has to read the header on rank 1..n, i.e., rank 0 has to write the
header before any other data is written.

## Issues

### 1: Shall we write the list of GIDs from rank 0?

_Resolved: Yes_

Pro: Does not change the current storage.
Con: Might be more cumbersome for application to use, as gids have to be
  aggregated at rank 0. Memory (a few MB).
