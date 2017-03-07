/* Copyright (c) 2014-2017, EPFL/Blue Brain Project
 *                          Juan Hernando Vieites <jhernando@fi.upm.es>
 *                          Raphael Dumusc <raphael.dumusc@epfl.ch>
 *                          Ahmet Bilgili <ahmet.bilgili@epfl.ch>
 *
 * This file is part of Brion <https://github.com/BlueBrain/Brion>
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 3.0 as published
 * by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef BRION_SPIKEREPORT_H
#define BRION_SPIKEREPORT_H

#include <brion/api.h>
#include <brion/types.h>

#include <boost/noncopyable.hpp>

#include <future>

namespace brion
{
namespace detail
{
class SpikeReport;
}

/** Read/writes access to a spike report.
 *
 * Following RAII, this class is ready to use after the creation and will ensure
 * release of resources upon destruction.
 *
 * There are two type of SpikeReports, depending on the semantics of the data
 * source:
 * - Static reports: The full spike data base is made available at
 *   construction time. This is the mode used by Bluron and NEST report
 *   file readers.
 * - Stream based reports: Spikes are read from a network stream. The stream
 *   always moves forward in time. The reader cannot steer or control how the
 *   source produces the spikes. Spikes are cached internally and made
 *   available by calling read/readUntil
 *
 * This class is not thread-safe except where stated otherwise.
 */
class SpikeReport
{
public:
    /**
     * The State enum : describe the state of the report
     * State::ok means that the report is ready to be read/written
     * State::ended means that the report end is reached (in read mode)
     * State::failed means that a error occured when readyn or writing the report
     */
    enum class State
    {
        ok,
        ended,
        failed
    };

    /**
     * Create a SpikeReport object given a URI.
     *
     * @param uri URI to spike report. The report type is deduced from
     *        here. The report types with built-in support are:
     *        - Bluron ('dat' extension), Bluron file based reports.
     *        - NEST ('gdf' extension). NEST file based reports. In read mode,
     *          shell wildcards are accepted at the file path leaf to load
     *          multiple report files.
     *        - Binary ('spikes' extension)
     *        Support for additional types can be added through plugins; see
     *        SpikeReportPlugin for the details.
     *
     * @param mode the brion::AccessMode bitmask. The report can be open only in
     *        brion::MODE_READ or brion::MODE_WRITE modes.
     *
     * @throw std::runtime_error if the input URI is not handled by any
     *        registered spike report plugin.
     * @version 2.0
     */
    BRION_API SpikeReport( const URI& uri, int mode = MODE_READ );

    /**
     * Open a report in read mode with a subset selection.
     *
     * @param uri thre port uri
     * @param subset The set of gids to be reported. This set should be
     *        understood as a filter: any included GID which is not
     *        actually part of the report will be silently ignored when reading
     *        data.
     * @version 2.0
     */
    BRION_API SpikeReport( const URI& uri, const GIDSet& ids );

    /**
     * Release all resources.
     * Any pending read/seek tasks will be interrupted and calling
     * get on the corresponding future will throw std::runtime_error.
     */
    BRION_API ~SpikeReport();

    BRION_API SpikeReport( SpikeReport&& );
    BRION_API SpikeReport& operator = ( SpikeReport&& );

    /** @return the descriptions of all loaded report backends. @version 1.10 */
    BRION_API static std::string getDescriptions();

    /**
     * Close the report. The close operation is blocking and it interrupts
     * all the pending read/seek operations.
     * @version 2.0
     */
    BRION_API void close();

    /**
     * \return true if the report was closed
     * @version 2.0
     */
    BRION_API bool isClosed() const;

    /**
     * Interrupt any pending read/seek operation.
     * This method is blocking.
     * @version 2.0
     */
    BRION_API void interrupt();

    /**
    * Get the URI used to instantiate the report. It could be different from
    * the input URI, depending on the plugin implementation.
    *
    * @return The URI used in the instance. It could be the same as the input
    * URI or a different one, depending on the implementation
    * @version 2.0
    */
    BRION_API const URI& getURI() const;

    /**
     * @return the end time of the latest complete read/write operation or
     *         0 if no operation has been issued.
     *
     * Read operations are deemed as complete when the returned future is
     * ready.
     *
     * The time inverval to which getCurrentTime refers is open on the
     * right. That means that upon completion of a read or write operation
     * no spike read or written may have a timestamp >= getCurrentTime().
     * @version 2.0
     */
    BRION_API float getCurrentTime() const;

    /**
     * @return the end timestamp of the report. This is the timestamp of the
     *         last spike known to be available or written or larger if the
     *         implementation has more metadata available.
     *         For stream reports this time is 0 before any operation is
     *         completed.
     * @version 2.0
     */
    BRION_API float getEndTime() const;

    /**
     * @return The state after the last completed operation.
     * @version 2.0
     */
    BRION_API State getState() const;

    /**
     * Read spikes until getCurrentTime becomes > min, the end of the report
     * is reached or the report closed.
     *
     * This function fetches all the available data from the source.
     * If no error occurs and the end is not reached, the min value passed
     * is also guaranteed to be inside the time window of the data returned.
     *
     * Preconditions:
     * - r.getState() is State::ok
     * - The report was open in read mode.
     * - There is no previous read or seek operation with a pending
     *   future.
     *
     * Postconditions:
     * Let:
     *  - r be the SpikeReport
     *  - f be the returned future by r.read(min)
     *  - and s = r.getCurrentTime() before read is called:
     * After f.wait() returns the following are all true:
     *  - r.getCurrentTime() >= s
     *  - If r.getState() == State::ok, then r.getCurrentTime() > min
     *  - For each spike timestamp t_s in f.get(),
     *    s <= t_s < r.getCurrentTime() (Note this could collapse to an
     *    empty interval if r.getState() != State::ok)
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
     * @note Until the completion of this operation, the internal state of the SpikeReport
     * may change.
     * @version 2.0
     */
    BRION_API std::future< Spikes > read( float min );

    /**
     * Read spikes until getCurrentTime() >= max, the end of the report is
     * reached or the report closed.
     *
     * This function is very similar to the normal read, but instead of
     * specifying a lower bound of getCurrentTime at return, it specifies
     * a requested strict value. The preconditions and postconditions
     * are the same except for those involving the parameter min which
     * become:
     *
     * Precondition: max > getCurrentTime()
     * Postcondition: If r.getState() == State::ok, then
     *                r.getCurrentTime() >= max
     * @throw std::runtime_error if the precondition does not hold.
     * @note Until the completion of this operation, the internal state of the SpikeReport
     * may change.
     * @sa seek()
     * @version 2.0
     */
    BRION_API std::future< Spikes > readUntil( float max );

    /**
     * Seek to a given absolute timestamp.
     *
     * If toTimestamp >= getCurrentTime() and the report was open for reading,
     * data will be skipped forward until the timestamp is made current. In write
     * mode for streams, consumers are notified about the new timestamp.
     *
     * The case toTimestamp < getCurrentTime() is only supported by file
     * based reports.
     * In write mode, seeks are only supported in binary reports. Forward seeking
     * simply updates getCurrentTime() and backward seeking followed by a write,
     * will overwrite the existing data.
     *
     * If the seek operation is not supported, a std::runtime_error will be
     * thrown if this method is called.
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
     * @note Until the completion of this operation, the internal state of the SpikeReport
     * may change.
     * @version 2.0
     */
    BRION_API std::future< void > seek( float toTimeStamp );

    /**
     * Write the given spikes to the output.
     * Preconditions:
     * - spikes is a sorted by time stamp
     * - spikes.front().first < getCurrentTime()
     *
     * Upon return getCurrenTime() is the greatest of all the spike times
     * plus an epsilon.
     *
     * @param spikes A collection of spikes sorted by timestamp in ascending
     *        order. For every spike, its timestamp must be >= getCurrentTime().
     * @throw std::runtime_error if the report is read-only,
     * @throw std::logic_error if a precondition does not hold
     * @version 2.0
     */
    BRION_API void write( const Spikes& spikes );

    /**
     * @return Whether the report supports seek to t < getCurrentTime() or not.
     * @version 2.0
     */
    BRION_API bool supportsBackwardSeek() const;

private:
    std::unique_ptr< detail::SpikeReport > _impl;

    SpikeReport( const SpikeReport& ) = delete;
    SpikeReport& operator = ( const SpikeReport& ) = delete;
};

inline std::ostream& operator<<( std::ostream& stream,
                                 const brion::SpikeReport::State state )
{
    switch ( state )
    {
    case brion::SpikeReport::State::ok:
        return stream << "ok";
    case brion::SpikeReport::State::ended:
        return stream << "ended";
    case brion::SpikeReport::State::failed:
        return stream << "failed";
    default:
        return stream;
    }
}
}

#endif
