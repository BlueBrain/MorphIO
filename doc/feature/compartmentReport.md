Additions to compartmentReport to support ZeroEQ streams {#compartmentReport}
============

## Motivation

The current CompartmentReport lacks an implementation based on ZeroEQ as the
SpikeReport does. There is a network plugin implementated with codash, but it
has been deprecated. Since there's a new specification of the SpikeReport API
ready for implementation, it makes sense to review the CompartmentReport
interface to make it consistent with the new SpikeReport and prepare it for the
ZeroEQ restrictions at the same time.

This specification proposes a new interface for the CompartmentReport with
these goals in mind.

## Proposed API changes:

### API changes:

- The constructors are a bit inconsistent. The proposed change is to remove the
  deprecated one and the write only constructor and replace the read/write
  constructor by these two overloads:

       /** Open a compartment report
        *
        * In read mode, this function may block if needed until all the
        * information has been collected (e.g. the report metadata).
        *
        * @param uri URI to compartment report. The report type is deduced from
        *        here.
        * @param mode the brion::AccessMode bitmask
        * @param gids the neurons of interest in READ_MODE
        * @throw std::runtime_error if source is not a valid compartment report
        *        or could not be open in the mode requested.
        * @version TBD
        */
       CompartmentReport( const URI& uri, const int mode )

       /*
        * Open a report in read mode with a subset selection.
        *
        * This function may block if needed until all the information has
        * been collected (e.g. the report metadata).
        *
        * @param uri URI to compartment report. The report type is deduced from
        *        here.
        * @param subset The set of gids to be reported. This set should be
        *        understood as a filter.
        * @throw std::runtime_error if source is not a valid compartment report
        *        or could not be open.
        * @version TBD
        */
       CompartmentReport( const URI& uri, const GIDSet& subset );

  These constructors are also consistent with the new SpikeReport.

- Add the following functions:

       /**
        * @return The current position in the report. This is the start time
        *         to use in write operations and the next read operation.
        * @version TBD
        */
       float getCurrentTime() const;

       enum State { OK = 0, ENDED = 1, FAILED = 2 };
       /**
        * @return The state after the last completed operation.
        * @version TBD
        */
       State getState() const;

       /**
        * Seek to a given absolute timestamp.
        *
        * In stream based reports it is only possible to seek forward.
        *
        * Preconditions:
        * - r.getState() is OK or ENDED.
        * - The report was open in read mode.
        * - There is no previous read or seek operation with a pending future.
        *
        * Postconditions:
        * Let:
        *  - r be the CompartmentReport
        *  - f be the returned future by r.seek(toTimestamp)
        *  After f.wait() returns:
        *  - r.getCurrentTime() ~= toTimestamp rounded to the nearest multiple
             of the timestep, ...
        *  - ... getState() == OK -> r.getEndTime() > getCurrentTime() and ...
        *  - ... getState() == ENDED -> r.getEndTime() == getCurrentTime()
        *
        * After successful f.wait_for or f.wait_until, the result is the same
        * as above. If they time out, no observable state changes.
        *
        * @throw std::logic_error if a precondition is not met or the operation
        *        is not supported by the implementation.
        * @version TBD
        */
       boost::future< void > seek( const float toTimestamp );

       /** Finish writing the current frame and advance to the next one.
        *
        * Upon return getCurrentTime() has been increased to the nearest
        * multiple of the timestep greater than the value before.
        */
       void endFrame();

- Replace loadFrame(float) by readNextFrame():

       /**
        * Read the next compartment frame.
        *
        * Preconditions:
        * - r.getState() is OK or ENDED.
        * - The report was open in read mode.
        * - There is no previous read or seek operation with a pending future.
        *
        * Postconditions:
        * Let:
        *  - r be the CompartmentReport
        *  - f be the returned future by r.readNextFrame()
        *  - and s = r.getCurrentTime() before read is called (s = -inf if
        *    undefined):
        * After f.wait() returns the following are all true:
        *  - If r.getState() == ENDED s == r.getCurrentTime() and f.get() is
        *    empty
        *  - Otherwise r.getCurrentTime() ~= s + r.getTimestep() (the timestamp
        *    is rounded to the nearest multiple of the timesetp).
        *  - The compartments counts and offsets becomes defined.
        *
        * After successful f.wait_for or f.wait_until, the result is the same
        * as above. If they time out, no observable state changes.
        *
        * @throw std::logic_error if a precondition is not met.
        * @version TBD
        */
       boost::future< floatsPtr > readNextFrame( );

- Remove updateMapping(). The cell set may only be specified in the constructor.
  Note also the the word mapping has been completely removed from the API and
  its documentation.

- Current write functions have inconsistent return types: writeHeader is void
  whereas writeCompartments, writeFrame and flush return bool. The proposal
  is to make all new write functions void and use exceptions to report write
  errors instead. This is consistent with read functions using exceptions to
  report errors.

- Remove the timestamp argument from writeFrame to enforce sequential writing.
  If random access is needed for any reason client code could still use seek
  for that purpose. The motivation for this change is the simplification of
  the stream implementation while making it easier to detect unsupported
  operations (e.g. seek backwards in a stream). To avoid confusion, writeFrame
  is renamed to writeCell. The new function is:

       /** Write the voltage for once cell at a point in time.
        *
        * Preconditions:
        * - The report was open in write mode.
        * - writeCellCompartments has already been called for all cells to be
        *   written.
        *
        * @param gid the GID of the cell
        * @param voltages the voltages per compartment to save
        * @throw std::logic_error if a precondition is not met and
        *        std::runtime_eror if an IO error occurs.
        * @version TBD
        */
       void writeCell( const uint32_t gid, const floats& voltages );

  Replace writeCompartments with writeCellCompartments, with a new return type
  and additional preconditions:

       /** Write the compartment counts for each section for one cell.
        *
        * Preconditions:
        * - The report was open in write mode.
        * - writeHeader has already been called.
        *
        * @param gid the GID of the cell
        * @param counts the number of compartments per section
        * @throw std::logic_error if a precondition is not met and
        *        std::runtime_eror if an IO error occurs.
        * @version TBD
        */
       void writeCellCompartments( const uint32_t gid,
                                   const uint16_ts& counts );


- Remove the following functions: getBufferSize(), setBufferSize(),
  clearBuffer(). These functions can be provided by a high level interface
  if needed.

- Remove getFrameSize(), this function doesn't seem to be needed and it's
  complicated to define its behaviour in the stream case.

### Documentation:

- Add the following paragraphs to the class description:

        (between 2nd and 3rd)

        * A compartment report is a discrete sampling in time and space of a
        * continuous scalar variable over a cell population. Time is discretized
        * in frames and space is discretized in compartments. Compartments are
        * subdivisions of cells' sections, each section is divided up into n
        * equally sized compartments longitudinally. A frame is the sampling of
        * the values that the report variable takes at each compartment at a
        * precise point in time. Frames are evenly spaced by the report timestep
        * (ts). In the simplest interpolation scheme, let f(t) = x be the value
        * of the report variable at a compartment at frame n (t = n * ts), f(t +
        * e) = x for every e in [0, timestep).

        (after last)

        * Thread-safey is only guaranteed for const access. That also means that
        * accessing the const& types returned by some functions is not
        * thread-safe while a non const operation is called or any returned
        * future is being waited.

- Replace the documentation of getGIDs() with this:

       /** @return In read mode, return the GIDSet that applies to the result
        *          of the last read operation. Note that this set may be a
        *          subset of the GIDSet given at construction (that is the case
        *          if the report does not contains all cells at origin). In
        *          write mode it returns the GID set that has been set using
        *          writeCellCompartments.
        */

- Replace the documentation of mapping functions with this new one

  For getOffsets():

       /** Get the current section offsets of each neuron in the simulation
        *  frame buffer.
        *
        * For instance, getOffsets()[1][15] retrieves the lookup index for the
        * frame buffer for section 15 of neuron with index 1. The neuron index
        * is derived from the order in the GID set returned by getGIDs().
        *
        * The result is empty until the first frame is read.
        *
        * For requested cells that are not part of the report its section
        * offset list will be empty.
        *
        * @return the offset for each section for each neuron
        */

  For getCompartmentCounts():

       /** Get the number of compartments for each section of each neuron
        *  provided.
        *
        * The neuron index is derived from the order in the GID set returned
        * by getGIDs().
        *
        * The result is empty until the first frame is read.
        *
        * @return the compartment counts for each section for each neuron
        * @version 1.0
        */
       const CompartmentCounts& getCompartmentCounts() const;

 getNumCompartments():

       /** Get the number of compartments for the given neuron.
        *
        * The neuron index is derived from the order in the GID set returned
        * by getGIDs().
        *
        * @param index neuron index per current GID set
        * @return number of compartments for the given neuron or 0 if the
        *         report has just been created and no frame has been read yet.
        */

  Note that the new functions do not return valid data until the first frame
  is read.

- Replace getEndTime() documentation by:

       /** @return the most up to date end time of the report.
        *
        * This value may change upon the completion of readNextFrame, seek,
        * or writeHeader.
        */
       float getEndTime() const;

- Add the following preconditions to writeHeader:

        * - The report was open in write mode.
        * - The header has not already been written.

## Implementation

This section will cover some implementation details related to the stream
case as the file based IO is straight forward.

The most two important decisions taken are:
- The granularity of the messages sent won't be the whole frame but a cell.
- The mapping will be always included as part of the frame.

The rationale for these decisions follows.

The current API for writing is already cell oriented, while this is convenient
to write on memory mapped files or H5 files without requiring the user to
assemble the whole buffer before writing, it's not consistent with the fact that
reads are whole frame oriented. This means that in the stream case we have to
assemble the frame at the writer or reader part of the implementation, or
change the API and force the user to do it on the writer side. Requiring
the user to assemble the frame is the easiest solution for the implementation
of the stream based case but it requires bigger changes to adapt existing code
both in the user side and in the implementation of existing plugins.
Therefore, the decision here is to keep the API similar to the current one
and send one message per cell. This allows interesting things like filtering
cell subsets on origin based on topics, something much more difficult to do
with whole frames. It also allows very easily to have multiple clients receiving
different portions of the circuit and only those. Obviuosly this introduces an
overhead, but we expect it to be small in relative and absolute terms. At the
same time we don't expect to stream full compartment data for detailed
morphologies for more than 10^5 cells at the moment. The implementation will
be revised if problems arise trying to do point to point streaming of larger
data sets. Having a separate message per cell also simplifies the creation of
the global mapping provided at the reader, because it can be created locally
based on the selected cell subset regardless of which cells are being
published by the writer. That said, soma only reports may receive special
treatment for efficiency reasons, but that probably implies an API extension
also.

The reason for sending the mapping as part of every (per cell) frame is also
simplicity. If the mapping is a separate piece of information that needs to
be requested by clients, it adds complexity in several points. The first one
is the constructor, that instead of simply creating a subscriber, needs to
request the mapping synchronously. Changing the cell set of interest on the
reader is also more complicated, since it requires again a request that with
the current API must be synchronous. The proposed approach also avoids having
to add a finishMapping function to know then the mapping is complete and can
be sent to receivers potentially waiting in their constructors.
Related to the issue above we could distinguish between receiving a global
mapping or per cell mappings. If the mapping was requested separately, having
a whole frame, makes all the code more complicated. The overhead of including
the per cell mapping inside the data message can be estimated knowing the
maximum and average numbers of comparments section (unreported sections need
to be included in this calculation). For example, assuming at most 16
compartments per section and 4 compartments on average, the overhead is an
additional 1/32th of the frame size. In this case, for every 4 float numbers
(the compartment data) we have to add a compartment count that can be enconded
with 4 bits, in other words we have 4/(4*32) extra bits for each data bit. The
actual overhead still needs to be figured out, but anything around this 3%
seems reasonable, overall considering that is data size is a problem a big
reduction can be achived using compression of the floating point data.

The messages of the protocol are:
 - Publisher to subscribers:
   * FRAME_DATA
   * FRAME_BARRIER
   * REPORT_METADATA
 - Subscribers to publisher:
   * REQUEST_METADATA

The fields of a frame data message are the following:
 - cell GID
 - timestamp
 - compartment data (n floats)
 - compartment counts (m integers packed as 4 or 8 bit n = Sum counts)

The timestamp is guaranteed to be the same for all cells in the same frame which
means that a frame can be considered finished when the last received timestamp
is different to the previous one. Nevertheless, in order to avoid receivers wait
until the first message of the next frame arrives, the FRAME_BARRIER message
is used to signal the end of a frame (this message is an empty event). The
FRAME_BARRIER message is used before the very first frame data message, this
is done because otherwise receivers do not have enough information to know
if the very first frame is a complete frame or they joined late.

This protocol does not allows negative timestep in a stateless manner, bute
this can be easily addressed adding a timestep field to the message or a bit
to indicate the stream direction.

The timestamp could be sent as a separate message emitted when the first cell
in a frame is sent, but this has the disadvantage of making the protocol
stateful and in case of traffic congestion the results would be incorrect if
that message is dropped.

## Examples

## Issues

### 1: How does a the ZeroEQ implementation get the metadata from the subscriberat construction time?

_Resolved: No_

It will have to be by discovery of the data source via broadcasting, unless a
request/reply mechanism is implemented in ZeroEQ. The problem of this
broadcasting is that any publisher could answer while the report can be
created with a specific URI to connect to. This really looks like something
that should be implemented via request reply.

### 2: Subset selection should be performed at the receiver or sender side?

_Resolved: No_

This is undecided yet, but the specification of the API doesn't rely ony any
of the approaches and the protocol can support both.

### 3: Why selectCellSet() can make getCurrentTime() advance and miss data?

_Resolved: No_

The reason is simple, but it's a scenario that is exclusive of the stream case.
When frames are received they may have been already reduced to a given GID set
at origin or even locally. If selectCellSet requests a GID set which is not a
subset of the current one, the frames received but not yet consumed are not
valid for the new mapping, so they have to be thrown away. This efectively
makes the current time advance, even if the frames have not been returned.

An alternative could be to make selectCellSet a completely asynchronous
request with a future, and then make the client responsible of checking if
the mapping has changed after every successful read operation. This alternative
can only be efficient if the report provides a hash of the mapping to avoid
expensive comparisons.

Another solution is to remove this function and require creating a new reader
in order to change the mapping. This may be a good solution in the context of
the stream source, but maybe not be so good for files.

### 4: Why getCurrentTime returns the start time of the next read operation instead of the last one?

_Resolved: Yes_

For consistency with seek. If seek jumps to a point in the report, it seems
reasonable to make getCurrentTime return this value after completion. This is
only possible if getCurrentTime is referred to the next read operation. After
a read operation has been completed, the only thing needed to find out the
start time is to subtract the timestep.

This is symmetric to SpikeReport::getCurrentTime(), the only disadvantage is
that the timestamp returned is not really part of the time window of the
the last frame read because [start, end) is open on the right. Admitedly, this
can be confusing, but the only reasonable alternative seems to be to return the
timestamp read along the data in readNextFrame.

### 5: Some changes are breaking the API, should we bump the version number to 2?

_Resolved: No_

### 6: Should the FRAME_BARRIER message include a timestamp for robustness?

_Resolved: No_

The discussion may be deferred after the initial implementation is finished.