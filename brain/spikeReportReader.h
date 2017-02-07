
/* Copyright (c) 2006-2015, Raphael Dumusc <raphael.dumusc@epfl.ch>
 *                          Juan Hernando <jhernando@fi.upm.es>
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


#ifndef BRAIN_SPIKEREPORTREADER_H
#define BRAIN_SPIKEREPORTREADER_H

#include <brain/api.h>
#include <brain/types.h>

#include <boost/noncopyable.hpp>

namespace brain
{
/**
 * Reader for Spike data.
 *
 * Following RAII, all readers are ready for use after the creation and will
 * ensure release of resources upon destruction.
 *
 * This class is not thread-safe except where noted.
 */
class SpikeReportReader : public boost::noncopyable
{
public:
    /**
     * Construct a new reader opening a spike data source.
     * @param uri URI to spike report (can contain a wildcard to specify several
     * files).
     * @version 1.0
     * @throw std::runtime_error if source is invalid.
     */
    BRAIN_API explicit SpikeReportReader( const brion::URI& uri );

    /**
     * Construct a new reader opening a spike data source.
     * @param uri URI to spike report (can contain a wildcard to specify several
     * files).
     * @param subset Subset of cells to be reported.
     * files).
     * @version 1.0
     * @throw std::runtime_error if source is invalid.
     */
    BRAIN_API SpikeReportReader( const brion::URI& uri, const GIDSet& subset );

    /**
     * Destructor.
     * @version 1.0
     */
    BRAIN_API ~SpikeReportReader();

    /**
     * Get all spikes inside a time window.
     *
     * For stream reports this method will wait until the first spike
     * with a time larger or equal to end arrives. The time interval is
     * open on the right, so assuming that spikes arrive in order, this
     * method will return a full snapshot of the spikes between [start, end).
     * Precondition : start < end
     * @throw std::logic_error if the precondition is not fulfilled.
     * @version 1.0
     */
    BRAIN_API Spikes getSpikes( const float start, const float end );

    /**
     * @return the end timestamp of the report. This is the timestamp of the
     *         last spike known to be available or larger if the implementation
     *         has more metadata available.
     *         For stream reports this time is 0 and it is updated when
     *         getSpikes is called.
     * @version 1.0
     */
     BRAIN_API float getEndTime() const;

    /**
     * @return true if any of the versions of getSpikes() reaches the end
     *         of the stream, if the report is static or if closed has been
               called.
     * @version 1.0
     */
    BRAIN_API bool hasEnded() const;

    /**
     * Close the data source.
     *
     * Any thread blocked in getSpikes will return immediately, possibly
     * returning an empty container.
     * This method may be called concurrently to both getSpikes() functions
     * and hasEnded().
     *
     * @version 1.0
     */
    BRAIN_API void close();

private:
    class _Impl;
    _Impl* _impl;
};

}
#endif
