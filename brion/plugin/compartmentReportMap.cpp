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

#include <lunchbox/atomic.h>
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
#ifdef BRION_USE_OPENMP
const size_t _nThreads = omp_get_num_procs();
#else
const size_t _nThreads = 1;
#endif
}

namespace plugin
{
namespace
{
    lunchbox::PluginRegisterer< CompartmentReportMap > registerer;
}

CompartmentReportMap::CompartmentReportMap(
                                const CompartmentReportInitData& initData )
    : _uri( std::to_string( initData.getURI( )) + "_" )
    , _readable( false )
{
    // have at least one store
    const auto& uri = initData.getURI();
    _stores.emplace_back( keyv::Map( uri ));
    _stores.back().setQueueDepth( _queueDepth );
    if( uri.getScheme() == "memcached" ) // parallelize loading with memcached
        while( _stores.size() < _nThreads )
        {
            _stores.emplace_back( keyv::Map( uri ));
            _stores.back().setQueueDepth( _queueDepth );
        }

    const int accessMode = initData.getAccessMode();

    if(( accessMode & MODE_READ ) && !_loadHeader( ))
        LBTHROW( std::runtime_error( "Incomplete or missing report at " +
                                     std::to_string( uri )));

    if( accessMode == MODE_WRITE || accessMode == MODE_READWRITE )
    {
        if( _loadHeader( ))
            LBTHROW( std::runtime_error( "Cannot overwrite existing report at "+
                                         std::to_string( uri )));
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
    for( auto& store : _stores )
        store.setByteswap( false );
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
    return _stores.front().insert(
        _uri + countsKey + std::to_string( gid ), counts );
}

bool CompartmentReportMap::writeFrame( const uint32_t gid,
                                       const floats& voltages,
                                       const float time )
{
    if( !_flushHeader( ))
        return false;

#ifndef NDEBUG
    const uint16_ts& counts =
        _stores.front().getVector< uint16_t >( _uri + countsKey +
                                               std::to_string( gid ));
    const size_t size = std::accumulate( counts.begin(), counts.end(), 0 );
    LBASSERTINFO( size == voltages.size(), "gid " << gid << " should have " <<
                  size << " voltages not " << voltages.size( ));
#endif

    const size_t index = _getFrameNumber( time );
    const std::string& key = _uri + std::to_string( gid ) + "_" +
                             std::to_string( index );
    return _stores.front().insert( key, voltages );
}

bool CompartmentReportMap::flush()
{
    if( !_flushHeader( ))
        return false;
    for( auto& store : _stores )
        store.flush();
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

    auto& store = _stores.front();
    if( !store.insert( _uri + headerKey, _header ) ||
        !store.insert( _uri + gidsKey, _gids ) ||
        !store.insert( _uri + dunitKey, _dunit ) ||
        !store.insert( _uri + tunitKey, _tunit ))
    {
        return false;
    }

    LBVERB << "Wrote meta information of " << _uri << std::endl;
    return _loadHeader();
}

bool CompartmentReportMap::_loadHeader()
{
    const auto gids = _gids; // keep requested gids
    _clear();
    _gids = gids; // restore after clear

    try
    {
        auto& store = _stores.front();
        _header = store.get< Header >( _uri + headerKey );
        const bool byteswap = ( _header.magic != _magic );
        if( byteswap )
        {
            lunchbox::byteswap( _header );
            for( auto& store_ : _stores )
                store_.setByteswap( true );
        }

        LBASSERT( _header.magic == _magic );
        if( _header.magic != _magic )
        {
            LBWARN << "report header has wrong magic " << _header.magic
                   << " instead of " << _magic << std::endl;
            _clear();
            return false;
        }

        if( _header.version != _version )
        {
            LBWARN << "report has version " << _header.version
                   << ", can only read version " << _version << std::endl;
            _clear();
            return false;
        }
        LBASSERTINFO( _header.endTime - _header.startTime >= _header.timestep,
                      "Invalid report time " << _header.startTime << ".." <<
                      _header.endTime << "/" << _header.timestep );

        const bool loadGIDs = _gids.empty();
        store.fetch( _uri + dunitKey );
        store.fetch( _uri + tunitKey );
        if( loadGIDs )
            store.fetch( _uri + gidsKey, _header.nGIDs * sizeof( uint32_t ));
        for( const uint32_t gid : _gids )
            store.fetch( _uri + countsKey + std::to_string( gid ));

        _dunit = store[ _uri + dunitKey ];
        _tunit = store[ _uri + tunitKey ];
        if( loadGIDs )
            _gids = store.getSet< uint32_t >( _uri + gidsKey );

        if( _gids.empty( ))
        {
            LBWARN << "No gids for report" << std::endl;
            _clear();
            return false;
        }

        Strings keys;
        keys.reserve( _gids.size( ));
        std::unordered_map< std::string, uint32_t > gidMap;
        for( const uint32_t gid : _gids )
        {
            keys.push_back( _uri + countsKey + std::to_string( gid ));
            gidMap.emplace( keys.back(), gid );
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

        store.takeValues( keys, takeValue );
        if( taken != _gids.size( ))
        {
            LBWARN << "Missing mapping " << _gids.size() - taken << " of "
                   << _gids.size() << " gids" << std::endl;
            _clear();
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

        _readable = true;
        return true;
    }
    catch( const std::runtime_error& e )
    {
        LBWARN << "Missing header information: " << e.what() << std::endl;
        _clear();
        return false;
    }

    return _readable;
}

void CompartmentReportMap::updateMapping( const GIDSet& gids )
{
    if( _gids == gids && !gids.empty() && _readable )
        return;

    const auto all = _stores.front().getSet< uint32_t >( _uri + gidsKey );
    const auto& subset = gids.empty() ? all : gids;

    _gids = all.empty() ? subset : _computeIntersection( all, subset );

    if( _gids.empty( ))
        LBTHROW( std::runtime_error( "CompartmentReportMap::updateMapping:"
                                     " GIDs out of range" ));
    if( !_loadHeader( ))
        LBTHROW( std::runtime_error( "Incomplete data source" ));
}

floatsPtr CompartmentReportMap::loadFrame( const float time ) const
{
    if( !_readable )
        return floatsPtr();

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
        keys.push_back( _uri + std::to_string( gid ) + index );
        offsetMap.emplace( keys.back(), offset );
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

#ifdef BRION_USE_OPENMP
    lunchbox::a_ssize_t taken;
    omp_set_num_threads( _stores.size( ));
#else
    size_t taken = 0;
#endif
#pragma omp parallel
    {
#ifdef BRION_USE_OPENMP
        auto& store = _stores[ omp_get_thread_num( )];
        const size_t start = float( omp_get_thread_num( )) *
                          float( keys.size( )) / float( omp_get_num_threads( ));
        const size_t end = float( omp_get_thread_num() + 1 ) *
                          float( keys.size( )) / float( omp_get_num_threads( ));
        const Strings subKeys( keys.begin() + start, keys.begin() + end );
#else
        auto& store = _stores.front();
        const Strings& subKeys = keys;
#endif

        const auto takeValue = [ ptr, &offsetMap, &taken ]
            ( const std::string& key, char* data, const size_t size )
        {
            ::memcpy( ptr + offsetMap[ key ], data, size );
            std::free( data );
            ++taken;
        };

        store.takeValues( subKeys, takeValue );
    }

    if( size_t( taken ) == _gids.size( ))
        return buffer;

    LBWARN << "Missing " << _gids.size() - taken << " of " << _gids.size()
           << " gids in report frame at " << time << " ms" << std::endl;
    return floatsPtr();
}

}
}
