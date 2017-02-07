
/* Copyright (c) 2006-2017, Raphael Dumusc <raphael.dumusc@epfl.ch>
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

#include <brion/spikeReport.h>

#include <lunchbox/log.h>

namespace brain
{

class SpikeReportReader::_Impl
{
public:
    explicit _Impl( const brion::URI& uri )
        : _report( uri, brion::MODE_READ )
    {}

    _Impl( const brion::URI& uri, const GIDSet& subset )
        : _report( uri, subset )
    {}

    brion::SpikeReport _report;
    brion::Spikes _collected;
};

SpikeReportReader::SpikeReportReader( const brion::URI& uri )
    : _impl( new _Impl( uri ))
{
}

SpikeReportReader::SpikeReportReader( const brion::URI& uri,
                                      const GIDSet& subset )
    : _impl( new _Impl( uri, subset ))
{
}

SpikeReportReader::~SpikeReportReader()
{
    delete _impl;
}

Spikes SpikeReportReader::getSpikes( const float startTime,
                                     const float endTime )
{
    if( endTime <= startTime )
        LBTHROW( std::logic_error(
                     "Start time should be strictly inferior to end time" ));

    if( _impl->_report.supportsBackwardSeek( ))
    {
        _impl->_report.seek( startTime ).get();
        return _impl->_report.readUntil( endTime ).get();
    }

    // In reports that don't support seek we just want to move forward at
    // least until end time, we'll find the window later.
    // We use read instead of readUntil so the end time gets updated with
    // the latest value possible. We also try to read always, even if all spikes
    // in the requested window have been already collected.
    auto& collected = _impl->_collected;
    auto spikes = _impl->_report.read( endTime ).get();
    if( !spikes.empty( ))
    {
        collected.reserve( collected.size() + spikes.size( ));
        collected.insert( collected.end(), spikes.begin(), spikes.end( ));
    }

    return Spikes(
        std::lower_bound( collected.begin(), collected.end(), startTime,
                          []( const Spike& spike, const float val ){
                              return spike.first < val; }),
        std::lower_bound( collected.begin(), collected.end(), endTime,
                          []( const Spike& spike, const float val ){
                              return spike.first < val; }));
}

float SpikeReportReader::getEndTime() const
{
    return _impl->_report.getEndTime();
}

bool SpikeReportReader::hasEnded() const
{
    return _impl->_report.getState() == brion::SpikeReport::State::ended;
}

void SpikeReportReader::close()
{
    _impl->_report.close();
}

}
