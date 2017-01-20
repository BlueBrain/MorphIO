
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

#include <boost/noncopyable.hpp>
#include <brion/types.h>

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
    explicit SpikeReportReader( const brion::URI& uri );

    /**
     * Destructor.
     * @version 1.0
     */
    ~SpikeReportReader();

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
    brion::Spikes getSpikes( const float start, const float end );

    /**
     * @return true if any of the versions of getSpikes() reaches the end
     *         of the stream, if the report is static or if closed has been
               called.
     * @version 1.0
     */
    bool hasEnded() const;


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
    void close();

private:
    class _Impl;
    _Impl* _impl;
};

}
#endif
