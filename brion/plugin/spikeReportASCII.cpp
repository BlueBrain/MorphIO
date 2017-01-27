/* Copyright (c) 2017, EPFL/Blue Brain Project
 *                     Mohamed-Ghaith Kaabi <mohamed.kaabi@epfl.ch>
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

#include "spikeReportASCII.h"

#include "../pluginInitData.h"

#include <boost/filesystem.hpp>
#include <boost/regex.hpp>

#include <lunchbox/pluginRegisterer.h>
#include <lunchbox/stdExt.h>

#include <fstream>

namespace brion
{
namespace plugin
{

SpikeReportASCII::SpikeReportASCII( const SpikeReportInitData& initData )
    : SpikeReportPlugin( initData )
    , _lastReadPosition( _spikes.begin( ))
{
}

Spikes SpikeReportASCII::read( const float )
{
    // In file based reports, this function reads all remaining data.
    Spikes spikes;
    auto start = _lastReadPosition;
    _lastReadPosition = _spikes.end();
    _currentTime = UNDEFINED_TIMESTAMP;
    _state = State::ended;

    for( ; start != _spikes.end(); ++start )
        pushBack( *start, spikes );

    return spikes;
}

Spikes SpikeReportASCII::readUntil( const float toTimeStamp )
{
    Spikes spikes;
    auto start = _lastReadPosition;

    _lastReadPosition = std::lower_bound(
        _lastReadPosition, _spikes.end(), toTimeStamp,
        []( const Spike& spike, float val ) { return spike.first < val; });

    if( _lastReadPosition != _spikes.end( ))
         _currentTime = _lastReadPosition->first;
    else
    {
        _currentTime = UNDEFINED_TIMESTAMP;
        _state = State::ended;
    }

    if( start != _spikes.end( ))
    {
        std::for_each( start, _lastReadPosition,
                       [&spikes, this]( const Spike& spike )
                       {
                           pushBack( spike, spikes );
                       });
    }
    return spikes;
}

void SpikeReportASCII::readSeek( const float toTimeStamp )
{
    if( _spikes.empty( ))
    {
        _currentTime = UNDEFINED_TIMESTAMP;
        _state = State::ended;
        return;
    }

    if( toTimeStamp < _spikes.begin()->first )
    {
        _lastReadPosition = _spikes.begin();
        _state = State::ok;
        _currentTime = toTimeStamp;
    }
    else if( toTimeStamp > _spikes.rbegin()->first )
    {
        _lastReadPosition = _spikes.end();
        _state = State::ended;
        _currentTime = brion::UNDEFINED_TIMESTAMP;
    }
    else
    {
        _lastReadPosition = std::lower_bound(
            _spikes.begin(), _spikes.end(), toTimeStamp,
            []( const Spike& spike, float val ){ return spike.first < val; });
        _state = State::ok;
        _currentTime = toTimeStamp;
    }
}

void SpikeReportASCII::writeSeek( const float toTimeStamp )
{
    if( toTimeStamp < _currentTime )
        LBTHROW(
            std::runtime_error( "Backward seek not supported in write mode" ));

    _currentTime = toTimeStamp;
}

}
}
