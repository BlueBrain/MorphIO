High-Level CompartmentReport API for Python {#compartment_report_py}
============

## Motivation

Redesign the current BBPSDK-inspired API for also allowing voltage traces.
The returned data should change to be a matrix[frame, compartment] instead
of the current vector[compartments].

## API

### C++

* rename brain::CompartmentReportReader to CompartmentReport
* merge brion::CompartmentReport::loadFrame and loadNeuron in load( startTime, endTime )
  * returns matrix in frame-major order
* Change all timestamps to double

### Python

    class CompartmentReportView:
        def load()
            """Load all the available data from this view.

            If the report is empty return None, otherwise return a tuple with a 1D
            double ndarray for the frame timestamps and a 2D float ndarray, where
            the first index is for frames and the second for compartment offsets."""

        def load(start, end):
            """Load all frames in a time range.

            This function will wait if necessary until the requested time window becomes
            available. The return value is the same as for load()."""

        def load(timestamp):
            """Load the frame that contains the given timestamp.

            The caller will block until the frame on which the requested timestamp falls
            is available.
            The return type is a (double, nadarray) tuple where the first element is the
            actual timestamp of the frame and the second one is a 1D double ndarray."""

## Example

    import brain

    # voltage trace:
    reader = brain.CompartmentReport("uri")
    view = reader.create_view([1])
    times, trace = view.load()
    for time, frame in zip(times, trace):
        print "{}, {}".format(time, frame[soma_index(42)])

    # full frame of sub-target
    view = reader.create_view(range(1, 1000))
    time, frame = view.load( 42.f )
    for gid in range(1, 1000):
        print "{}, {}".format(gid, frame[soma_index(gid)])

    def soma_index(gid):
        return view.mapping.offsets[mapping.gids.index(gid)][0]

## Issues

### 1: Do we need separate functions for frame-based and trace-based access?

_Resolution: No_

The current CRView::load( start, end ) API already allows selecting a single
Neuron and loading all frames. The brion API will need to change (see above).

### 2: Do we provide column-major memory layout?

_Resolution: No_

We will not reformat the data layout to be column-major, i.e., packing all
samples of a single compartment together. This is an optimization and we do
not expect today to load more than a few neurons for voltage traces.

### 3: Should we change the time stamps to double?

_Resolution: Yes_

Reasons: Not memory critical since it's not stored much. Python uses double too,
and with plasticity simulations this becomes critical. This will create some
intermediate inconsistency with other classes.
