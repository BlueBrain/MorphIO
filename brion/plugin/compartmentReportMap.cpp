/* Copyright (c) 2014-2015, EPFL/Blue Brain Project
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
#define ASYNC_IO

std::string _getScope( const URI& uri )
{
    return uri.findQuery( "name" )->second + "_" +
        uri.findQuery( "target" )->second + "_";
}

template< class T > std::string toString( const T& value )
{
    return boost::lexical_cast< std::string >( value );
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
    return lunchbox::PersistentMap::handles( initData.getURI() )
            && !initData.getURI().getScheme().empty();
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
    return _store.insert( _getScope( _uri ) + countsKey + toString( gid ),
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
                                                            toString( gid ));
    const size_t size = std::accumulate( counts.begin(), counts.end(), 0 );
    LBASSERTINFO( size == voltages.size(), "gid " << gid << " should have " <<
                  size << " voltages not " << voltages.size( ));
#endif

    const size_t index = _getFrameNumber( time );
    const std::string& key = scope + toString( gid ) + "_" + toString( index );
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
#ifdef ASYNC_IO
        _store.fetch( scope + dunitKey );
        _store.fetch( scope + tunitKey );
        if( loadGIDs )
            _store.fetch( scope + gidsKey, _header.nGIDs * sizeof( uint32_t ));
        for( const uint32_t gid : _gids )
            _store.fetch( scope + countsKey + toString( gid ));
#endif

        _dunit = _store[ scope + dunitKey ];
        _tunit = _store[ scope + tunitKey ];
        if( loadGIDs )
            _gids = _store.getSet< uint32_t >( scope + gidsKey );

        _readable = true;
#ifdef ASYNC_IO
        if( loadGIDs )
            for( const uint32_t gid : _gids )
                _store.fetch( scope + countsKey + toString( gid ));
#endif

        uint64_t offset = 0;
        for( const uint32_t gid : _gids )
        {
            _cellCounts[ gid ] = _store.getVector< uint16_t >( scope +
                                                               countsKey +
                                                               toString( gid ));
            _counts.push_back( _cellCounts[ gid ] );

            const size_t nSections = _cellCounts[ gid ].size();
            _offsets.push_back( uint64_ts( nSections,
                                        std::numeric_limits<uint64_t>::max( )));

            for( size_t i = 0; i < nSections; ++i )
            {
                const uint16_t numCompartments = _cellCounts[ gid ][ i ];
                if( numCompartments == 0 )
                    continue;

                _offsets.back()[ i ] = offset;
                offset += numCompartments;
            }

            _totalCompartments += std::accumulate( _cellCounts[ gid ].begin(),
                                                   _cellCounts[ gid ].end(), 0);
        }
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
    floatsPtr buffer( new floats( getFrameSize( )));
    floats::iterator iter = buffer->begin();
    const size_t index = _getFrameNumber( time );
    std::map< uint32_t, size_t > sizeMap;

    for( const uint32_t gid : _gids )
    {
        const CellCompartments::const_iterator i = _cellCounts.find( gid );
        if( i == _cellCounts.end( ))
        {
            LBWARN << "Missing mapping for gid " << gid << std::endl;
            return floatsPtr();
        }
        const size_t size = std::accumulate( i->second.begin(),
                                             i->second.end(), 0 ) *
                            sizeof( float );
        sizeMap[ gid ] = size;

#ifdef ASYNC_IO
        _store.fetch( scope + toString( gid ) + "_" + toString( index ), size );
#endif
    }

    for( const uint32_t gid : _gids )
    {
        LBASSERTINFO( iter < buffer->end(), buffer->size() << " gid " << gid );
        const std::string& cellData = _store[ scope + toString( gid ) + "_" +
                                              toString( index ) ];
        LBASSERT( !cellData.empty( ));
        if( cellData.empty( ))
        {
            LBWARN << "Missing data for gid " << toString( gid ) << std::endl;
            return floatsPtr();
        }
        LBASSERTINFO( sizeMap[ gid ] == cellData.size(),
                      sizeMap[ gid ] << " != " << cellData.size( ));

        ::memcpy( &(*iter), cellData.data(), cellData.size( ));
        iter += cellData.size() / sizeof( float );
        if( iter > buffer->end( ))
            return floatsPtr();
    }
    if( iter == buffer->end( ))
        return buffer;

    LBASSERT( iter == buffer->end( ));
    return floatsPtr();
}

}
}
