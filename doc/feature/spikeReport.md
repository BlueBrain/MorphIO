Redesign of the SpikeReport and Spikes container {#spikeReport}
============

The current design of the SpikeReport has several flaws that need to be
addressed:
* It doesn't allow partial loading for file based reports.
* It exposes too many details of the stream case which can probably be hidden.
* The container class is a std::multimap while it could simply be a sorted array
  because no insertion is needed.
This specification proposes a new API to address these defficiencies as well
as include new requirements that need to be fulfilled.

## Requirements

The high level requirements for the replacement are listed below. Note that
all these requirements do not need to be directly satisfied by
brion::SpikeReport, but it must be possible to do so by a higher level class
based on it.

* Provide a container for spikes which is sorted by timestamp, storage
  efficient, and random accessible at the <sup>n</sup>th position in amortized
  constant time (provided the data has been already fetched). This is basically
  a std::vector, but only read access is needed.

* Allow partial data loading for file access.

* Allow the selection of a subset of the cells present in the data source to be
  reported. In stream-based reports this information should be made available
  to the sender to allow filtering at the source. In the ZeroEQ implementation,
  the same channel needed for communicating the desired target could be used
  for the handshaking required to ensure that the first spikes are not missed.

* As few details as possible that are specific to streams should be exposed.
  This means that it should be easy to write client code that works the same
  for both file based and stream based reports without special conditions for
  stream based reports.

* For stream reports the user will be able to:
 * Retrieve all the data until the end of the stream in a simple loop without
   doing any active wait (blocking calls are allowed).
 * Know if the end of stream has been reached.
 * Get all the data received so far without locking.
 * Get information about the latest timestamp reached by the source in the
   sender side even when there are no spikes at all (currently if the spike
   report is completely empty it's not possible to make any progress until the
   producer reaches the end of the report).

* When data for a time interval is requested, the time intervals are always
  closed on the left and open on the right.

## General remarks

It has been agreed that unifying both the file and the stream cases under
a stream-like interface (in the sense of std::iostream or POSIX file descriptor
IO) seems to be the best abstraction at the lower level. Providing random
access and out-of-core support is complicated at the lower level because
internal buffering is needed and this can only be implementated efficiently if
some assumptions about how client code will use the API are made. The most
important characteristic of the current proposal is that read and write
operations can only move forward, so each operation starts where the previous
one left over. This way there is no need to make any distinction between the
file and stream based cases. Random access is partially supported by a
seek function that for stream based reports will only support forward skip.

The current proposal defers important design decisions to the higher level API,
but the its benefit is that it provides a relatively simply API for the most
critical part.

## API

Thread-safey is only guaranteed for const access.

### Helper types

    struct Spike
    {
        Spike( float time_, uint32_t gid_ );
        float time;
        uint32_t gid;
    };

    typedef std::vector< Spike > Spikes;

### SpikeReport

    SpikeReport : public boost::noncopyable
    {
    public:
        SpikeReport( const URI& uri, int mode );

        /*
         * Open a report in read mode with a subset selection.
         *
         * @param uri
         * @param subset The set of gids to be reported. This set should be
         *        understood as a filter: any included GID which is not
         *        actually part of the report will be silently ignored.
         */
        SpikeReport( const URI& uri, const GIDSet& subset );

        /**
         * Release all resources.
         * Any threads waiting on futures will return immediately and calling
         * get will throw std::runtime_error.
         */
        ~SpikeReport();

        const URI& getURI();

        /** ? */
        float getStartTime() const;
        float getEndTime() const;

        /**
         * @return the end time of the latest complete read/write operation or
         *         UNDEFINED_TIMESTAMP if no operation has been issued.
         *
         * Read operations are deemed as complete when the returned future is
         * ready.
         *
         * The time inverval to which getCurrentTime refers is open on the
         * right. That means that upon completion of a read or write operation
         * no spike read or written may have a timestamp >= getCurrentTime().
         */
        float getCurrentTime() const;

        /**
         * Read spikes until getCurrentTime becomes > min, the end of the report
         * is reached or the report closed.
         *
         * This function fetches all the available data from the source.
         * If no error occurs and the end is not reached, the min value passed
         * is also guaranteed to be inside the time window of the data returned.
         *
         * Preconditions:
         * - r.getState() is OK or ENDED.
         * - The report was open in read mode.
         * - There is no previous read or seek operation with a pending
         *   future.
         * - min >= getCurrentTime() or UNDEFINED_TIMESTAMP.
         *
         * Postconditions:
         * Let:
         *  - r be the SpikeReport
         *  - f be the returned future by r.read(min)
         *  - and s = r.getCurrentTime() before read is called (s = -inf if
         *    undefined):
         * After f.wait() returns the following are all true:
         *  - r.getCurrentTime() >= s
         *  - If r.getState() == OK, then r.getCurrentTime() > min
         *  - For each spike timestamp t_s in f.get(),
         *    s <= t_s < r.getCurrentTime() (Note this could collapse to an
         *    empty interval if r.getState() != OK)
         *
         * After successful f.wait_for or f.wait_until, the result is the same
         * as above. If they time out, no observable state changes.
         *
         * If the state is FAILED or some other operation is still pending
         * when read is called the result is undefined.
         *
         * @param min The minimum end time until which spikes will be read. If
         *        UNDEFINED_TIMESTAMP, it will be considered as -infinite. This
         *        means that as much data as possible will be fetched without
         *        a minimum.
         * @throw std::logic_error if one of the preconditions is not fulfilled.
         */
        boost::future< Spikes > read( float min );

        /**
         * Read spikes until getCurrentTime() == max, the end of the report is
         * reached or the report closed.
         *
         * This function is very similar to the normal read, but instead of
         * specifying a lower bound of getCurrentTime at return, it specifies
         * a requested strict value. The preconditions and postconditions
         * are the same except for those involving the parameter min which
         * become:
         *
         * Precondition: max > getCurrentTime()
         * Postcondition: If r.getState() == OK, then r.getCurrentTime() == max
         *
         * @throw std::runtime_error if the precondition does not hold.
         * @sa seek()
         */
        boost::future< Spikes > readUntil( float max );

        /**
         * Seek to a given absolute timestamp.
         *
         * If toTimestamp >= getCurrentTime() and the report was opening
         * for reading, data will be skipped forward until the timestamp
         * is made current. In write mode for streams, consumers are notified
         * about the progress.
         *
         * The case toTimestamp < getCurrentTime() is only supported by file
         * based reports.
         *
         * Preconditions:
         * - There is no standing read or readUntil operation.
         *
         * Postconditions:
         * Let:
         *  - r be the SpikeReport
         *  - f be the returned future by r.seek(toTimestamp)
         * Then:
         *  - After f.wait() returns r.getCurrentTime() == toTimestamp.
         *  - The postconditions of read operations imply that in forward skips
         *    this function throws away the data previous to toTimestamp (or
         *    avoids reading it at all if possible).
         *
         * @throw std::runtime_error if a precondition does not hold or the
         *        operation is not supported by the implementation.
         */
        boost::future< void > seek( float toTimestamp );

        enum State { OK = 0, ENDED = 1, FAILED = 2 };
        /** @return The state after the last completed operation. */
        State getState() const;

       /**
         * Write the given spikes to the output.
         *
         * Upon return getCurrenTime() is the greatest of all the spike times
         * plus an epsilon.
         *
         * @param spikes A sorted collection of spikes. For every spike, its
         *        timestamp t must be >= getCurrentTime().
         */
        void write( const Spikes& spikes );
    }


## Examples

### Copying a spike report from one URL to another without any timeout.

    SpikeReport input( "url1", brion::MODE_READ )
    SpikeReport output( "url2", brion::MODE_WRITE )

    while( input.getState() == OK )
        output.write( input.read().get() ))

    if (input .getState() == SpikeReport::FAILED)
       ... // Report error.
    // To be strictly correct, the last operation should be output.seek to
    // advance until the current timestamp.

### Copying a spike report from one URL to another with a read timeout.

    SpikeReport intput( "url1", brion::MODE_READ )
    SpikeReport output( "url2", brion::MODE_WRITE )

    while( input.getState() == OK )
    {
        auto future = input.read();
        const auto status =
            future.wait_for( boost::chrono::milliseconds( 250 ));
        if( status == boost::future_state::ready )
            output.write( future.get( ));
        else
            ... // Handle timeout

        output.write( spikes );
        if (output.getState() == SpikeReport::FAILED)
           ... // Report error.
    }
    if (input.getState() == SpikeReport::FAILED)
       ... // Report error.

### Writing data at regular time intervals

    const float time delta = ...;
    for( size_t i = 0; ...; ++i )
    {
        const start = i * delta;
        const end = (i + 1) * delta;
        const Spikes = produceSpikes( start, end );
        output.write( spikes );
        output.seek( end ).get();
    }


## Implementation

TODO

## Issues


### 1: Do we need a Spikes container?

_Resolved: Yes_

Not at this level. A higher level class can could provide a custom container
if that's necessary for performance reasons. At this class, the interface is
designed in a way that the implementation does not need to cache anything under
normal conditions (no operation times out). This means that the container
can be handed over to the consumer and rely on RVO and move semantics to
avoid unnecessary copies.

### 2: Why Spikes::getCurrentTime() returns a time greater than the largest spike time (in the general case)?

_Resolved: Yes_

To ensure consistency with the specification of the time interval from
SpikeReport::read, which is open on the right. The time of the last spike is
still easy to recover because it is the last element in the container.

### 3: The return value of read should be boost::future< Spikes& > or boost::future< Spikes >?

_Resolved: No_

It is desirable to avoid the reference and according to
[this reference](http://en.cppreference.com/w/cpp/thread/future/get), it seems
that get() returns a meaningful value only the first time is called when
returning by value (because the implementation uses std::move, but I'm not
fully sure about this).

### 4: Are the getStartTime() and getEndTime() methods needed?

_Resolved: No_

Seems convenient, but it's not clear what value should they return when the
data source is a stream and no data (or metadata) has been received yet.

