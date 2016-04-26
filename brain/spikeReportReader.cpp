
/* Copyright (c) 2006-2015, Raphael Dumusc <raphael.dumusc@epfl.ch>
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

#include "spikeReportReader.h"
#include "spikes.h"
#include "detail/spikes.h"

#include <brion/spikeReport.h>

#include <math.h> // for nextafterf and INFINITY

namespace brain
{

/**
 * @internal
 *
 * Private subclass of Spikes.
 *
 * Spikes has to be subclassed here because of the protected constructor.
 * @see getSpikes()
 */
class BrionSpikes : public Spikes
{
public:
    BrionSpikes( brion::Spikes::const_iterator beginIt,
                 brion::Spikes::const_iterator endIt,
                 const float startTime,
                 const float endTime,
                 size_t count )
        : Spikes( new detail::Spikes( beginIt, endIt,
                                      startTime, endTime, count ))
    {}
};

class SpikeReportReader::_Impl
{
public:
    explicit _Impl( const brion::URI& uri )
        : _report( uri, brion::MODE_READ )
    {}

    brion::SpikeReport _report;
};

SpikeReportReader::SpikeReportReader( const brion::URI& uri )
    : _impl( new _Impl( uri ))
{
}

SpikeReportReader::~SpikeReportReader()
{
    delete _impl;
}

Spikes SpikeReportReader::getSpikes()
{
    brion::SpikeReport& report = _impl->_report;
    if( report.getReadMode() == brion::SpikeReport::STREAM )
    {
        if( report.getNextSpikeTime() == brion::UNDEFINED_TIMESTAMP )
        {
            // If the stream has reached the end (or an error ocurred) all
            // the spikes are fetched.
            report.waitUntil( brion::UNDEFINED_TIMESTAMP );
        }
        else
        {
            // Otherwise fetch all the spikes with time < last spike received.
            // Spikes with time == last time are left in the cache on
            // purpose. This ensures that the snapshot for [start, last time)
            // is complete.
            const float time = report.getLatestSpikeTime();
            // Ensuring that we don't block if no spikes have been received.
            if( time != brion::UNDEFINED_TIMESTAMP )
                report.waitUntil( nextafterf( time, -INFINITY ));
        }
    }

    return BrionSpikes( report.getSpikes().begin(), report.getSpikes().end(),
                        getStartTime(), getEndTime(),
                        report.getSpikes().size( ));
}

Spikes SpikeReportReader::getSpikes( const float startTime, const float endTime )
{
    const brion::Spikes& spikes = _impl->_report.getSpikes();

    if( endTime <= startTime )
        return BrionSpikes( spikes.end(), spikes.end(), startTime, endTime, 0 );

    // Receive spikes if needed
    if( _impl->_report.getReadMode() == brion::SpikeReport::STREAM )
    {
        // The interval is open on the right
        _impl->_report.waitUntil( nextafterf( endTime, -INFINITY ));
    }

    const brion::Spikes::const_iterator start = spikes.lower_bound(startTime);
    const brion::Spikes::const_iterator end = spikes.lower_bound(endTime);
    const size_t size = std::distance( start, end );

    return BrionSpikes( start, end, startTime, endTime, size );
}

bool SpikeReportReader::hasEnded() const
{
    return _impl->_report.getReadMode() == brion::SpikeReport::STATIC ||
           _impl->_report.getNextSpikeTime() == brion::UNDEFINED_TIMESTAMP;
}

bool SpikeReportReader::isStream() const
{
    return _impl->_report.getReadMode() == brion::SpikeReport::STREAM;
}

float SpikeReportReader::getStartTime() const
{
    return _impl->_report.getStartTime();
}

float SpikeReportReader::getEndTime() const
{
    return _impl->_report.getEndTime();
}

void SpikeReportReader::close()
{
    _impl->_report.close();
}

}
