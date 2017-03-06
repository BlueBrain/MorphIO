/* Copyright (c) 2015-2017, EPFL/Blue Brain Project
 *                          Stefan.Eilemann@epfl.ch
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

#include "compartmentReportDummy.h"

#include <lunchbox/pluginRegisterer.h>

#include <boost/lexical_cast.hpp>
#include <random>

namespace brion
{
namespace plugin
{
namespace
{
    lunchbox::PluginRegisterer< CompartmentReportDummy > registerer;
}

CompartmentReportDummy::CompartmentReportDummy(
    const CompartmentReportInitData& initData )
    : _randomValues( initData.getURI().findQuery("randomValues") !=
                     initData.getURI().queryEnd( ))
{
    if( initData.getAccessMode() != MODE_READ )
        return;

    // Prepare pseudo-random read data
    const auto& uri = initData.getURI();
    const auto& i = uri.findQuery("size");

    // we use <size> GIDs * ~1KB/GID compartments * 1024 frames
    _defaultGIDs = (i == uri.queryEnd( )) ? 1024 :
                       boost::lexical_cast< size_t >( i->second );
    _cacheNeuronCompartmentCounts( _gids );
}

std::string CompartmentReportDummy::getDescription()
{
    return R"(Benchmark drain (write): dummy://
Benchmark dummy source (read): dummy://[?size=size-in-MB][&randomValues]
    default 1024MB of dummy report data)";
}

const std::string& CompartmentReportDummy::getDataUnit() const
{
    static const std::string dunit = "unobtanium";
    return dunit;
}

const std::string& CompartmentReportDummy::getTimeUnit() const
{
    static const std::string tunit = "lightmilliseconds";
    return tunit;
}

float CompartmentReportDummy::getStartTime() const
{
    return 0.f;
}

float CompartmentReportDummy::getEndTime() const
{
    return 1024.f;
}

float CompartmentReportDummy::getTimestep() const
{
    return 1.f;
}

void CompartmentReportDummy::updateMapping( const GIDSet& gids )
{
    if( _gids == gids && !gids.empty( ))
        return;

    std::random_device dev;
    std::mt19937_64 engine( dev( ));
    std::uniform_int_distribution < uint32_t > distribution;
    engine.seed( _defaultGIDs ); // reproducible randomness, please

    GIDSet all;
    while( all.size() < _defaultGIDs )
        all.insert( distribution( engine ));

    const auto& subset = gids.empty() ? all : gids;
    _gids = _computeIntersection( all, subset );
    _counts.clear();
    _offsets.clear();
    if( _gids.empty( ))
        return;

    // aim for ~1k compartments/cell
    static const size_t spread = 32;
    static const size_t maxCompartments = 256 /* x 4B/value */ - spread / 2;
    uint64_t offset = 0;

    while( _counts.size() < _gids.size() - 1 )
    {
        _counts.push_back( uint16_ts( ));
        _offsets.push_back( uint64_ts( ));

        for( size_t nCompartments = 0; nCompartments < maxCompartments;
             nCompartments += _counts.back().back( ))
        {
            _counts.back().push_back(
                uint16_t( distribution( engine ) % (spread-1) + 1 ));
            _offsets.back().push_back( offset );
            offset += _counts.back().back();
        }
    }

    // Last cell: fill up to desired size to correct random errors above
    const size_t targetSize = _gids.size() * 256 /* x 4B/value */;
    _counts.push_back( uint16_ts( ));
    _offsets.push_back( uint64_ts( ));
    while( offset < targetSize )
    {
        _counts.back().push_back(
            uint16_t( distribution( engine ) % (spread-1) + 1 ));
        _offsets.back().push_back( offset );
        offset += _counts.back().back();
    }

    // correct last entry
    _counts.back().back() -= (offset - targetSize);
    _offsets.back().back() -= (offset - targetSize);
}

size_t CompartmentReportDummy::getFrameSize() const
{
    size_t frameSize = 0;
    for( const auto& counts : _counts )
        frameSize += std::accumulate( counts.begin(), counts.end(), 0 );
    return frameSize;
}

floatsPtr CompartmentReportDummy::loadFrame( const float time ) const
{
    floatsPtr buffer( new floats( getFrameSize( )));
    if( _randomValues )
    {
        std::random_device dev;
        std::mt19937_64 engine( dev( ));
        std::uniform_int_distribution < int16_t > distribution;
        engine.seed( unsigned( time )); // reproducible randomness, please

        for( float& value : *buffer )
            value = float( distribution( engine )) / 1000.f;
    }
    return buffer;
}

}
}
