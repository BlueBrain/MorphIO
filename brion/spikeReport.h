/* Copyright (c) 2014-2015, EPFL/Blue Brain Project
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

namespace brion
{
namespace detail { class SpikeReport; }

/** Read access to a SpikeReport.
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
 *   available by calling waitUntil. The user can clear spikes stored inside
 *   a given time window.
 *   In this report type, getStartTime and getEndTime return the time window
 *   of the spikes that are available to the client.
 *
 *   Client code can implement a moving window using waitUntil() and
 *   getNextSpikeTime(). The conceived usage is to decide a window width, and
 *   call waitUntil() using getNextSpikeTime() + width.
 *
 *   The loop
 *   \code
 *       while( report.waitUntil( report.getNextSpikeTime( )))
 *           ;
 *   \endcode
 *   is guaranteed to always make progress until the end of the stream is
 *   reached.
 *
 * This class is not thread-safe except where stated otherwise.
 */
class SpikeReport : public boost::noncopyable
{
public:
    /**
     * A type to specify how spikes are read by this SpikeReport.
     * @version 1.4
     */
    enum ReadMode { STATIC = 0, STREAM };

    /**
     * Create a SpikeReport object given a URI.
     *
     * @param uri URI to spike report. The report type is deduced from
     *        here. The report types with built-in support are:
     *        - Bluron ('dat' extension), Bluron file based reports.
     *        - NEST ('gdf' extension). NEST file based reports. In read mode,
     *          shell wildcards are accepted at the file path leaf to load
     *          multiple report files.
     *        Support for additional types can be added through plugins; see
     *        SpikeReportPlugin for the details.
     *
     * @param mode the brion::AccessMode bitmask
     * @throw std::runtime_error if the input URI is not handled by any
     *        registered spike report plugin.
     * @version 1.4
     */
    BRION_API SpikeReport( const URI& uri, const int mode );

    /** Destructor. @version 1.3 */
    BRION_API ~SpikeReport();

    /** @return the descriptions of all loaded report backends. @version 1.10 */
    BRION_API static std::string getDescriptions();

    /**
     * Get the URI used to instantiate the report. It could be different from
     * the input URI, depending on the plugin implementation.
     *
     * @return The URI used in the instance. It could be the same as the input
     * URI or a different one, depending on the implementation
     * @version 1.6
     */
    BRION_API const URI& getURI() const;

    /**
     * @version 1.4
     */
    BRION_API ReadMode getReadMode() const;

    /**
     * Get the time of the first spike.
     * @return The time in milliseconds, or UNDEFINED_TIMESTAMP if there
     *         are no spikes.
     * @version 1.3
     */
    BRION_API float getStartTime() const;

    /**
     * Get the time of the last spike.
     * @return The time in milliseconds, or UNDEFINED_TIMESTAMP if there
     *         are no spikes.
     * @version 1.3
     */
    BRION_API float getEndTime() const;

    /**
     * Get the spike times and cell GIDs.
     *
     * In STREAM reports this method returns all the spikes than have been
     * moved from the receive cache.
     *
     * @version 1.3 */
    BRION_API const Spikes& getSpikes() const;

    /**
     * Writes the spike times and cell GIDs.
     *
     * @param spikes Spikes to write.
     * @throw std::runtime_error if invoked on spike readers.
     * @version 1.4 */
    BRION_API void writeSpikes( const Spikes& spikes );

    /**
     * Lock the caller until the first spike past the given time stamp arrives
     * or the network stream is closed by the source.
     *
     * This is the only function that updates the Spikes data set returned by
     * getSpikes() with the spikes received from the stream.
     *
     * @param timeStamp The spike time to wait for in milliseconds. Using
     *        UNDEFINED_TIMESTAMP will make this function wait until the end
     *        of the stream.
     * @param timeout An optional timeout in milliseconds.
     * @return true at the moment a spike with time stamp larger than the input
     *         parameter arrives. False if any of the events below occur
     *         before the desired timestamp arrives:
     *         - The timeout goes off.
     *         - The network stream is closed or reaches the end.
     *         - The report is closed.
     *
     * @throw std::runtime_error if invoked on STATIC readers.
     * @version 1.4
     */
    BRION_API bool waitUntil( const float timeStamp,
                              const uint32_t timeout = LB_TIMEOUT_INDEFINITE );

    /**
     * Return the time of the next spike available in the internal cache.
     *
     * @return undefined in STATIC reports. In STREAM reports there are several
     *         cases:
     *         - 0 if no spikes have been received.
     *         - The earliest spike time in milliseconds than has been
     *           received, but has not been digested by waitUntil() if the
     *           internal cache is not empty.
     *         - The latest timestamp that was extracted from the cache if
     *           the cache is empty.
     *         - UNDEFINED_TIMESTAMP if the end of the stream has been reached
     *           and the cache is empty.
     * @throw std::runtime_error if invoked on non STREAM writers.
     * @version 1.4
     */
    BRION_API float getNextSpikeTime();

    /**
     * Return the time of the latest spike that has been received.
     *
     * @return undefined in STATIC reports. Let t be the timestamp of the
     *         latest spike received, in STREAM reports it returns:
     *         - UNDEFINED_TIMESTAMP is no spike has been received
     *         - t if at least one spike has been received and the end
     *           of the stream has not been reahed.
     *         - An unspecified value x, such as x > t, if at least one
     *           spike has been received and the end of the stream has been
     *           reached.
     *
     *         In any case, the function waitUntil() is guaranteed to not
     *         block if it takes as input a valid timestamp smaller than
     *         the value returned by getLatestSpikeTime().
     * @throw std::runtime_error if invoked on non STREAM writers.
     * @version 1.4
     */
    BRION_API float getLatestSpikeTime();

    /**
     * Remove all spikes contained in the given time interval
     *
     * The purpose of this method is to implement a moving window on top of
     * this API.
     *
     * @param startTime The start point of the interval
     * @param endTime The end point, if smaller than startTime no operation is
     *        performed
     * @throw std::runtime_error is the operation is not supported by the
     *        reader.
     * @version 1.4
     */
    BRION_API void clear( const float startTime, const float endTime );

    /**
     * Closes the report.
     *
     * Only meaningful for STREAM based reports. For reports opened in write
     * mode it finishes the reporting. For reports opened in read mode it
     * disconnects from the source, any call waiting in waitUntil will be
     * unblocked.
     *
     * Implicitly called by the destructor. Calling any other function after
     * the report has been closed has undefined behavior.
     *
     * @version 1.4
     */
    BRION_API void close();

private:
    detail::SpikeReport* _impl;
};

}
#endif
