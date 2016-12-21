/* Copyright (c) 2014-2016, EPFL/Blue Brain Project
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

#include "compartmentReportMap.h"

#include <lunchbox/bitOperation.h>
#include <lunchbox/debug.h>
#include <lunchbox/pluginRegisterer.h>

namespace lunchbox
{
template<> inline void byteswap(
    brion::plugin::CompartmentReportMap::Header& header )
{
    byteswap( header.magic );
    byteswap( header.version );
    byteswap( header.nGIDs );
    byteswap( header.startTime );
    byteswap( header.endTime );
    byteswap( header.timestep );
}
}

namespace brion
{
namespace
{
const std::string countsKey( "cellCount" );
const std::string dunitKey( "dunit" );
const std::string frameKey( "frame" );
const std::string gidsKey( "gids" );
const std::string headerKey( "header" );
const std::string tunitKey( "tunit" );

const uint32_t _version = 3; // Increase with each change in a k/v pair
const uint32_t _magic = 0xdb;
const size_t _queueDepth = 32768; // async queue depth, heuristic from benchmark

std::string _getScope( const URI& uri )
{
    return uri.findQuery( "name" )->second + "_" +
        uri.findQuery( "target" )->second + "_";
}
}

namespace plugin
{
namespace
{
    lunchbox::PluginRegisterer< CompartmentReportMap > registerer;
}

CompartmentReportMap::CompartmentReportMap(
                                const CompartmentReportInitData& initData )
    : _uri( initData.getURI( ))
    , _store( initData.getURI( ))
    , _readable( false )
{
    _store.setQueueDepth( _queueDepth );
    if( _uri.findQuery( "name" ) == _uri.queryEnd( ))
        _uri.addQuery( "name", "default" );
    if( _uri.findQuery( "target" ) == _uri.queryEnd( ))
        _uri.addQuery( "target", "allCompartments" );

    const int accessMode = initData.getAccessMode();

    if(( accessMode & MODE_READ ) && !_loadHeader( ))
        LBTHROW( std::runtime_error( "Incomplete or missing report at " +
                                     _uri.getPath( )));

    if( accessMode == MODE_WRITE || accessMode == MODE_READWRITE )
    {
        if( _loadHeader( ))
            LBTHROW( std::runtime_error( "Cannot overwrite existing report at "+
                                         _uri.getPath( )));
        _clear(); // reset after loading header
    }

    if( accessMode & MODE_READ )
        _cacheNeuronCompartmentCounts( initData.getGids( ));
}

CompartmentReportMap::~CompartmentReportMap()
{
}

bool CompartmentReportMap::handles(const CompartmentReportInitData& initData )
{
    return keyv::Map::handles( initData.getURI( )) &&
           !initData.getURI().getScheme().empty();
}

void CompartmentReportMap::_clear()
{
    _readable = false;
    _store.setByteswap( false );
    _header = Header();
    _gids.clear();
    _offsets.clear();
    _counts.clear();
    _totalCompartments = 0;
    _cellCounts.clear();
}

CompartmentReportMap::Header::Header()
    : magic( _magic )
    , version( _version )
    , nGIDs( 0 )
    , startTime( 0.f )
    , endTime( 0.f )
    , timestep( 1.f )
{}

std::ostream& operator << ( std::ostream& os,
                            const CompartmentReportMap::Header& h )
{
    return os << "Time " << h.startTime << ".." << h.endTime << " delta "
              << h.timestep;
}

void CompartmentReportMap::writeHeader( const float startTime,
                                        const float endTime,
                                        const float timestep,
                                        const std::string& dunit,
                                        const std::string& tunit )
{
    LBASSERTINFO( endTime-startTime >= timestep,
                  "Invalid report time " << startTime << ".." << endTime <<
                  "/" << timestep );
    _clear();

    _header.startTime = startTime;
    _header.endTime = endTime;
    _header.timestep = timestep;
    _dunit = dunit;
    _tunit = tunit;
}

bool CompartmentReportMap::writeCompartments( const uint32_t gid,
                                              const uint16_ts& counts )
{
    LBASSERTINFO( !counts.empty(), gid );
    _gids.insert( gid );
    return _store.insert( _getScope( _uri ) + countsKey + std::to_string( gid ),
                          counts );
}

bool CompartmentReportMap::writeFrame( const uint32_t gid,
                                       const floats& voltages,
                                       const float time )
{
    if( !_flushHeader( ))
        return false;

    const std::string& scope = _getScope( _uri );
#ifndef NDEBUG
    const uint16_ts& counts = _store.getVector< uint16_t >( scope + countsKey +
                                                            std::to_string( gid ));
    const size_t size = std::accumulate( counts.begin(), counts.end(), 0 );
    LBASSERTINFO( size == voltages.size(), "gid " << gid << " should have " <<
                  size << " voltages not " << voltages.size( ));
#endif

    const size_t index = _getFrameNumber( time );
    const std::string& key = scope + std::to_string( gid ) + "_" + std::to_string( index );
    return _store.insert( key, voltages );
}

bool CompartmentReportMap::flush()
{
    if( !_flushHeader( ))
        return false;
    _store.flush();
    return true;
}

bool CompartmentReportMap::_flushHeader()
{
    if( _readable )
        return true;
    LBASSERTINFO( _header.endTime - _header.startTime >= _header.timestep,
                  "Invalid report time " << _header.startTime << ".." <<
                  _header.endTime << "/" << _header.timestep );

    _header.nGIDs = uint32_t(_gids.size( ));

    const std::string& scope = _getScope( _uri );
    if( !_store.insert( scope + headerKey, _header ) ||
        !_store.insert( scope + gidsKey, _gids ) ||
        !_store.insert( scope + dunitKey, _dunit ) ||
        !_store.insert( scope + tunitKey, _tunit ))
    {
        return false;
    }

    LBVERB << "Wrote meta information of " << _uri << std::endl;
    return _loadHeader();
}

bool CompartmentReportMap::_loadHeader()
{
    _clear();
    const std::string& scope = _getScope( _uri );

    try
    {
        _header = _store.get< Header >( scope + headerKey );
        const bool byteswap = ( _header.magic != _magic );
        if( byteswap )
        {
            lunchbox::byteswap( _header );
            _store.setByteswap( true );
        }

        LBASSERT( _header.magic == _magic );
        if( _header.magic != _magic )
        {
            LBWARN << "report header has wrong magic " << _header.magic
                   << " instead of " << _magic << std::endl;
            return false;
        }

        if( _header.version != _version )
        {
            LBWARN << "report has version " << _header.version
                   << ", can only read version " << _version << std::endl;
            return false;
        }
        LBASSERTINFO( _header.endTime - _header.startTime >= _header.timestep,
                      "Invalid report time " << _header.startTime << ".." <<
                      _header.endTime << "/" << _header.timestep );

        const bool loadGIDs = _gids.empty();
        _store.fetch( scope + dunitKey );
        _store.fetch( scope + tunitKey );
        if( loadGIDs )
            _store.fetch( scope + gidsKey, _header.nGIDs * sizeof( uint32_t ));
        for( const uint32_t gid : _gids )
            _store.fetch( scope + countsKey + std::to_string( gid ));

        _dunit = _store[ scope + dunitKey ];
        _tunit = _store[ scope + tunitKey ];
        if( loadGIDs )
            _gids = _store.getSet< uint32_t >( scope + gidsKey );

        _readable = true;
        if( !loadGIDs )
            return true;

        Strings keys;
        keys.reserve( _gids.size( ));
        std::unordered_map< std::string, uint32_t > gidMap;
        for( const uint32_t gid : _gids )
        {
            keys.push_back( scope + countsKey + std::to_string( gid ));
            gidMap[ keys.back() ] = gid;
        }

        size_t taken = 0;
        const auto takeValue = [&] ( const std::string& key,
                                     char* data, const size_t size )
        {
            const uint32_t gid = gidMap[ key ];
            _cellCounts[ gid ] = std::vector< uint16_t >(
                reinterpret_cast< const uint16_t* >( data ),
                reinterpret_cast< const uint16_t* >( data + size ));

            std::free( data );
            ++taken;
        };

        _store.takeValues( keys, takeValue );
        if( taken != _gids.size( ))
        {
            LBWARN << "Missing " << _gids.size() - taken << " of "
                   << _gids.size() << " gids in report header" << std::endl;
            _cellCounts.clear();
            return false;
        }

        uint64_t offset = 0;
        for( const uint32_t gid : _gids )
        {
            const auto& cellCounts = _cellCounts[ gid ];
            _counts.push_back( cellCounts );

            const size_t nSections = cellCounts.size();
            _offsets.push_back( uint64_ts( nSections,
                                        std::numeric_limits<uint64_t>::max( )));

            for( size_t i = 0; i < nSections; ++i )
            {
                const uint16_t numCompartments = cellCounts[ i ];
                if( numCompartments == 0 )
                    continue;

                _offsets.back()[ i ] = offset;
                offset += numCompartments;
            }

            _totalCompartments += std::accumulate( cellCounts.begin(),
                                                   cellCounts.end(), 0 );
        }
        return true;
    }
    catch( const std::runtime_error& e )
    {
        LBWARN << "Missing header information: " << e.what() << std::endl;
        return false;
    }

    return _readable;
}

void CompartmentReportMap::updateMapping( const GIDSet& gids )
{
    if( _gids == gids && !gids.empty() && _readable )
        return;

    _gids = gids;
    if( !_loadHeader( ))
        LBTHROW( std::runtime_error( "Incomplete data source" ));
}

floatsPtr CompartmentReportMap::loadFrame( const float time ) const
{
    if( !_readable )
        return floatsPtr();

    const std::string& scope = _getScope( _uri );
    const std::string& index = std::string( "_" ) +
                               std::to_string( _getFrameNumber( time ));

    floatsPtr buffer( new floats( getFrameSize( )));
    float* const ptr = buffer->data();

    std::unordered_map< std::string, size_t > offsetMap;
    size_t offset = 0;

    Strings keys;
    keys.reserve( _gids.size( ));

    for( const uint32_t gid : _gids )
    {
        keys.push_back( scope + std::to_string( gid ) + index );
        offsetMap[ keys.back() ] = offset;
        const CellCompartments::const_iterator i = _cellCounts.find( gid );
        if( i == _cellCounts.end( ))
        {
            LBWARN << "Missing mapping for gid " << gid << std::endl;
            return floatsPtr();
        }
        const size_t size = std::accumulate( i->second.begin(),
                                             i->second.end(), 0 );
        offset += size;
    }

    size_t taken = 0;
    const auto takeValue = [ ptr, &offsetMap, &taken ]
        ( const std::string& key, char* data, const size_t size )
    {
        ::memcpy( ptr + offsetMap[ key ], data, size );
        std::free( data );
        ++taken;
    };

    _store.takeValues( keys, takeValue );
    if( taken == _gids.size( ))
        return buffer;

    LBWARN << "Missing " << _gids.size() - taken << " of " << _gids.size()
           << " gids in report frame at " << time << " ms" << std::endl;
    return floatsPtr();
}

}
}
