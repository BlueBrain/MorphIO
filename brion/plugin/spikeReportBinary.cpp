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

#include "spikeReportBinary.h"

#include <lunchbox/memoryMap.h>
#include <lunchbox/pluginRegisterer.h>

#include <boost/filesystem/path.hpp>

#define BINARY_REPORT_FILE_EXT ".spikes"

namespace brion
{
namespace plugin
{
namespace
{
lunchbox::PluginRegisterer< SpikeReportBinary > registerer;
static const uint32_t magic = 0xf0a;
static const uint32_t version = 1;
}

SpikeReportBinary::SpikeReportBinary( const SpikeReportInitData& initData )
    : _uri( initData.getURI( ))
{
    if( initData.getAccessMode() & MODE_READ )
    {
        lunchbox::MemoryMap file( _uri.getPath( ));
        const size_t size = file.getSize();

        if( (size % sizeof( uint32_t )) != 0 )
            LBTHROW( std::runtime_error( "Incompatible binary report in " +
                                         initData.getURI().getPath( )));

        const size_t nElems = size / sizeof( uint32_t );
        const uint32_t* iData = file.getAddress< uint32_t >();
        const float* fData = file.getAddress< float >();
        size_t index = 0;

        if( index >= nElems || iData[ index++ ] != magic )
            LBTHROW( std::runtime_error( "Bad magic in " +
                                         initData.getURI().getPath( )));
        if( index >= nElems || iData[ index++ ] != version )
            LBTHROW( std::runtime_error( "Bad version in " +
                                         initData.getURI().getPath( )));

        // Inserting a range of sorted elements is faster that inserting one
        // by one.
        std::vector< Spike > spikes;
        spikes.reserve( (nElems-2)/2 );
        for( ; index+1 < nElems; index += 2 ) // uint32_t, float pairs
            spikes.push_back( Spike( fData[ index ], iData[ index + 1 ] ));

        // std::sort not needed, writer wrote sorted array
        _spikes.insert( spikes.begin(), spikes.end( ));
    }
}

bool SpikeReportBinary::handles( const SpikeReportInitData& initData )
{
    const URI& uri = initData.getURI();
    if( !uri.getScheme().empty() && uri.getScheme() != "file" )
        return false;

    const boost::filesystem::path ext =
        boost::filesystem::path( uri.getPath() ).extension();
    return ext == BINARY_REPORT_FILE_EXT;
}

std::string SpikeReportBinary::getDescription()
{
    return "Blue Brain binary spike reports:\n"
           "  [file://]/path/to/report" BINARY_REPORT_FILE_EXT;
}

const URI& SpikeReportBinary::getURI() const
{
    return _uri;
}

float SpikeReportBinary::getStartTime() const
{
    return _spikes.empty() ? std::numeric_limits< float >::max() :
                             _spikes.begin()->first;
}

float SpikeReportBinary::getEndTime() const
{
    return _spikes.empty() ? std::numeric_limits< float >::max() :
                             _spikes.rbegin()->first;
}

const Spikes& SpikeReportBinary::getSpikes() const
{
    return _spikes;
}

void SpikeReportBinary::writeSpikes( const Spikes& spikes )
{
    const size_t nElems = spikes.size();
    const size_t size = sizeof( magic ) + sizeof( version ) +
                        nElems * sizeof( float ) + nElems * sizeof( uint32_t );
    lunchbox::MemoryMap file( _uri.getPath(), size );

    uint32_t* iData = file.getAddress< uint32_t >();
    float* fData = file.getAddress< float >();
    size_t index = 0;

    iData[ index++ ] = magic;
    iData[ index++ ] = version;
    for( const Spike& spike : spikes )
    {
        fData[ index++ ] = spike.first;
        iData[ index++ ] = spike.second;
        LBASSERT( index <= size );
    }

    _spikes = spikes;
}

void SpikeReportBinary::close()
{
    _spikes.clear();
}

}
}
