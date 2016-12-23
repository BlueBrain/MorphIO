/* Copyright (c) 2013-2017, EPFL/Blue Brain Project
 *                          Daniel Nachbaur <daniel.nachbaur@epfl.ch>
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

#include "compartmentReportBinary.h"

#include <boost/filesystem/path.hpp>
#include <boost/foreach.hpp>
#include <lunchbox/debug.h>
#include <lunchbox/log.h>
#include <lunchbox/pluginRegisterer.h>
#include <lunchbox/stdExt.h>
#include <map>

namespace
{
// Offsets of the header information in the file.
enum HeaderPositions
{
    //! position of the double value identifying the byte order of the file
    IDENTIFIER = 0,
    //! offset of header information (int32_t) past the architecture identifier
    HEADER_SIZE = IDENTIFIER + sizeof(double),
    //! version of the reader library? (char *)
    LIBRARY_VERSION = 16,
    //! version of the simulator used in the simulation (char *)
    SIMULATOR_VERSION = 32,
    //! number of cells in each frame of the report (int32_t)
    TOTAL_NUMBER_OF_CELLS = 48,
    //! number of compartments in a frame (int32_t)
    TOTAL_NUMBER_OF_COMPARTMENTS = 52,
    //! number of frames in the report (int32_t)
    NUMBER_OF_STEPS = 64,
    //! time where the report starts in specified time unit (double)
    TIME_START = 72,
    //! time where the report ends in specified time unit (double)
    TIME_END = 80,
    //! timestep between two report frames (double)
    DT_TIME = 88,
    //! unit of the report data (char *)
    D_UNIT = 96,
    //! time unit of the report (char *)
    T_UNIT = 112,
    //! size of the mapping (int32_t)
    MAPPING_SIZE = 128,
    //! name of the mapping (char *)
    MAPPING_NAME = 144,
    //! size of the extra mapping (int32_t)
    EXTRA_MAPPING_SIZE = 160,
    //! name of the extra mapping (char *)
    EXTRA_MAPPING_NAME = 176,
    //! name of the report (char *)
    REPORT_NAME = 192,
    //! length of the header (int32_t)
    HEADER_LENGTH = 1024
};

// Offsets of data tokens inside a cell info block inside the file header.
enum CellInfoHeaderPositions
{
    // Cell_GID (int32_t)
    NUMBER_OF_CELL = 0,
    // number of compartments of this cell (int32_t)
    NUMBER_OF_COMPARTMENTS = 8,
    // data info offset (int32_t)
    DATA_INFO = 16,
    // extra mapping info offset (int32_t)
    EXTRA_MAPPING_INFO = 24,
    // mapping info offset (int32_t)
    MAPPING_INFO = 32,
    // size of the cellinfo block (int32_t)
    SIZE_CELL_INFO_LENGTH = 64
};

struct CellInfo
{
    int32_t gid;
    int32_t numCompartments;
    uint64_t mappingOffset;
    uint64_t extraMappingOffset;
    uint64_t dataOffset;

    bool operator < ( const CellInfo& rhs ) const
        { return gid < rhs.gid; }
};

typedef std::vector< CellInfo > CellInfos;

// If identifier read at position 0 matches ARCHITECTURE_IDENTIFIER, then the
// file was writting from native architecture
const double ARCHITECTURE_IDENTIFIER = 1.001;

template< typename T > T get( const uint8_t* buffer, const size_t offset )
{
    return *reinterpret_cast< const T* >( buffer + offset );
}

template< typename T > const T* getPtr( const uint8_t* buffer,
                                        const size_t offset )
{
    return reinterpret_cast< const T* >( buffer + offset );
}
}

namespace lunchbox
{
template<> inline
void byteswap( CellInfo& value )
{
    byteswap( value.gid );
    byteswap( value.numCompartments );
    byteswap( value.mappingOffset );
    byteswap( value.extraMappingOffset );
    byteswap( value.dataOffset );
}
}

namespace brion
{
namespace plugin
{

namespace
{
    lunchbox::PluginRegisterer< CompartmentReportBinary > registerer;
}

CompartmentReportBinary::CompartmentReportBinary(
        const CompartmentReportInitData& initData )
    : _startTime( 0 )
    , _endTime( 0 )
    , _timestep( 0 )
    , _file()
    , _header()
    , _subNumCompartments( 0 )
    , _subtarget( false )
{
    if( initData.getAccessMode() != MODE_READ )
        LBTHROW( std::runtime_error( "Writing of binary compartments not "
                                     "implemented" ));

    _file.map( initData.getURI().getPath( ) );

    if( !_parseHeader( ))
        LBTHROW( std::runtime_error( "Parsing header failed" ));

    if( !_parseMapping( ))
        LBTHROW( std::runtime_error( "Parsing mapping failed" ));

    _cacheNeuronCompartmentCounts( initData.getGids( ));

}

CompartmentReportBinary::~CompartmentReportBinary()
{
}

bool CompartmentReportBinary::handles(
        const CompartmentReportInitData& initData )
{
    if( initData.getAccessMode() != MODE_READ )
        return false;

    const URI& uri = initData.getURI();
    if( !uri.getScheme().empty() && uri.getScheme() != "file" )
        return false;

    const boost::filesystem::path ext =
            boost::filesystem::path( uri.getPath() ).extension();
    return ext == ".bin" || ext == ".rep" || ext == ".bbp";
}

const GIDSet& CompartmentReportBinary::getGIDs() const
{
    return _gids;
}

const SectionOffsets& CompartmentReportBinary::getOffsets() const
{
    return _offsets[_subtarget ? 1 : 0];
}

const CompartmentCounts& CompartmentReportBinary::getCompartmentCounts() const
{
    return _counts[_subtarget ? 1 : 0];
}

size_t CompartmentReportBinary::getFrameSize() const
{
    return _subtarget ? _subNumCompartments : _header.numCompartments;
}

floatsPtr CompartmentReportBinary::loadFrame( const float timestamp ) const
{
    const uint8_t* ptr = _file.getAddress< const uint8_t >();
    if( !ptr || _offsets[_subtarget].empty( ))
        return floatsPtr();

    const size_t frameNumber = _getFrameNumber( timestamp );
    const size_t frameOffset = _header.dataBlockOffset +
                          _header.numCompartments * sizeof(float) * frameNumber;

    if( !_subtarget )
    {
        floatsPtr buffer( new floats( _header.numCompartments ));
        memcpy( buffer->data(), ptr + frameOffset,
                _header.numCompartments * sizeof(float) );

        if( _header.byteswap )
        {
#pragma omp parallel for
            for( int32_t i = 0; i < _header.numCompartments; ++i )
                lunchbox::byteswap( (*buffer)[i] );
        }
        return buffer;
    }

    if( _subNumCompartments == 0 )
        return floatsPtr();

    floatsPtr buffer( new floats( _subNumCompartments ));
    const float* const source = (const float*)(ptr + frameOffset);
    const SectionOffsets& offsets = getOffsets();
    const CompartmentCounts& compCounts = getCompartmentCounts();

    for( size_t i = 0; i < _gids.size(); ++i )
    {
        for( size_t j = 0; j < offsets[i].size(); ++j )
        {
            const uint16_t numCompartments = compCounts[i][j];
            const uint64_t sourceOffset = _conversionOffsets[i][j];
            const uint64_t targetOffset = offsets[i][j];

            for( uint16_t k = 0; k < numCompartments; ++k )
                (*buffer)[k + targetOffset] = source[k + sourceOffset];
        }
    }

    if( _header.byteswap )
    {
#pragma omp parallel for
        for( ssize_t i = 0; i < ssize_t( _subNumCompartments ); ++i )
            lunchbox::byteswap( (*buffer)[i] );
    }
    return buffer;
}

floatsPtr CompartmentReportBinary::loadNeuron( const uint32_t gid ) const
{
    const uint8_t* const bytePtr = _file.getAddress< const uint8_t >();
    if( !bytePtr || _offsets[_subtarget].empty( ))
        return floatsPtr();

    const size_t index = std::distance( _gids.begin(), _gids.find( gid ));
    if( index >= _gids.size( ))
        return floatsPtr();

    const float* const ptr = (const float*)(bytePtr + _header.dataBlockOffset);

    const size_t frameSize = _header.numCompartments;
    const size_t nFrames = (_endTime - _startTime) / _timestep;
    const size_t nCompartments = getNumCompartments( index );
    const size_t nValues = nFrames * nCompartments;
    floatsPtr buffer( new floats( nValues ));

    const SectionOffsets& offsets = _offsets[0];
    const CompartmentCounts& compCounts = getCompartmentCounts();
    for( size_t i = 0; i < nFrames; ++i )
    {
        const size_t srcOffset = i * frameSize;
        size_t dstOffset = i * nCompartments;
        for( size_t j = 0; j < offsets[index].size(); ++j )
        {
            const uint16_t numCompartments = compCounts[index][j];
            const uint64_t sourceOffset = offsets[index][j];

            ::memcpy( buffer->data() + dstOffset,
                      ptr + srcOffset + sourceOffset,
                      numCompartments * sizeof( float ));
            dstOffset += numCompartments;
        }
    }

    if( _header.byteswap )
    {
#pragma omp parallel for
        for( ssize_t i = 0; i < ssize_t( nValues ); ++i )
            lunchbox::byteswap( (*buffer)[i] );
    }

    return buffer;
}

void CompartmentReportBinary::updateMapping( const GIDSet& gids )
{
    _gids = gids.empty() ? _originalGIDs : gids;
    _subtarget = (_gids != _originalGIDs);

    if( !_subtarget )
        return;

    GIDSet intersection = _computeIntersection( _originalGIDs, _gids );
    if( intersection.empty( ))
    {
        LBTHROW( std::runtime_error( "CompartmentReportBinary::updateMapping:"
                                     " GIDs out of range" ));
    }
    if( intersection != _gids )
    {
        updateMapping( intersection );
        return;
    }

    // build gid to mapping index lookup table
    stde::hash_map< uint32_t, size_t > gidIndex;
    size_t c = 0;
    for( GIDSetCIter i = _originalGIDs.begin(); i != _originalGIDs.end(); ++i )
        gidIndex[*i] = c++;

    _conversionOffsets.resize( gids.size( ));
    _counts[1].resize( gids.size( ));

    // then build conversion mapping from original to subtarget
    size_t cellIndex = 0;
    _subNumCompartments = 0;
    for( GIDSetCIter i = _gids.begin(); i != _gids.end(); ++i )
    {
        const size_t index = gidIndex[*i];

        uint64_ts& sectionOffsets = _conversionOffsets[cellIndex];
        uint16_ts& sectionCounts = _counts[1][cellIndex];

        const size_t numSections = _offsets[0][index].size();
        sectionOffsets.resize( numSections );
        sectionCounts.resize( numSections );

        for( uint16_t sid = 0; sid < numSections; ++sid )
        {
            const uint16_t compCount = _counts[0][index][sid];
            sectionOffsets[sid] = _offsets[0][index][sid];
            sectionCounts[sid] = compCount;
            _subNumCompartments += compCount;
        }
        ++cellIndex;
    }

    _offsets[1].resize( gids.size( ));

    cellIndex = 0;
    uint64_t offset = 0;
    for( GIDSetCIter i = _gids.begin(); i != _gids.end(); ++i )
    {
        uint64_ts& sectionOffsets = _offsets[1][cellIndex];
        uint16_ts& sectionCounts = _counts[1][cellIndex];

        sectionOffsets.resize( _offsets[0][gidIndex[*i]].size(),
                               std::numeric_limits< uint64_t >::max( ));

        // This assignment implies that report values will be resorted
        // according to section IDs when the information of the original
        // frame is copied into the subtarget frame
        for( size_t j = 0; j < sectionOffsets.size(); ++j )
        {
            const uint16_t numCompartments = sectionCounts[j];
            if( numCompartments == 0 )
                continue;
            sectionOffsets[j] = offset;
            offset += numCompartments;
        }
        ++cellIndex;
    }
}

void CompartmentReportBinary::writeHeader( const float /*startTime*/,
                                           const float /*endTime*/,
                                           const float /*timestep*/,
                                           const std::string& /*dunit*/,
                                           const std::string& /*tunit*/ )
{
    LBUNIMPLEMENTED;
}

bool CompartmentReportBinary::writeCompartments( const uint32_t /*gid*/,
                                                 const uint16_ts& /*counts*/ )
{
    LBUNIMPLEMENTED;
    return false;
}

bool CompartmentReportBinary::writeFrame( const uint32_t /*gid*/,
                                          const floats& /*voltages*/,
                                          const float /*timestamp*/ )
{
    LBUNIMPLEMENTED;
    return false;
}

bool CompartmentReportBinary::flush()
{
    LBUNIMPLEMENTED;
    return false;
}

bool CompartmentReportBinary::_parseHeader()
{
    const uint8_t* ptr = _file.getAddress< const uint8_t >();
    if( !ptr )
        return false;

    _header.identifier = get< double >( ptr, IDENTIFIER );
    _header.headerSize = get< int32_t >( ptr, HEADER_SIZE );
    _header.numCells = get< int32_t >( ptr, TOTAL_NUMBER_OF_CELLS );
    _header.numCompartments = get< int32_t >( ptr,
                                              TOTAL_NUMBER_OF_COMPARTMENTS );
    _header.libVersion = getPtr< char >( ptr, LIBRARY_VERSION );
    _header.simVersion = getPtr< char >( ptr, SIMULATOR_VERSION );
    _header.numFrames = get< int32_t >( ptr, NUMBER_OF_STEPS );
    _startTime = get< double >( ptr, TIME_START );
    _endTime = get< double >( ptr, TIME_END );
    _timestep = get< double >( ptr, DT_TIME );
    _dunit = getPtr< char >( ptr, D_UNIT );
    _tunit = getPtr< char >( ptr, T_UNIT );
    _header.mappingSize = get< int32_t >( ptr, MAPPING_SIZE );
    _header.mappingName = getPtr< char >( ptr, MAPPING_NAME );
    _header.extraMappingSize = get< int32_t >( ptr, EXTRA_MAPPING_SIZE );
    _header.extraMappingName = getPtr< char >( ptr, EXTRA_MAPPING_NAME );
    _header.reportName = getPtr< char >( ptr, REPORT_NAME );

    _header.byteswap = _header.identifier != ARCHITECTURE_IDENTIFIER;

    if( _header.byteswap )
    {
        lunchbox::byteswap( _header );
        lunchbox::byteswap( _startTime );
        lunchbox::byteswap( _endTime );
        lunchbox::byteswap( _timestep );

        if( _header.identifier != ARCHITECTURE_IDENTIFIER )
        {
           LBERROR << "File is corrupt or originated from an unknown "
                   << "architecture." << std::endl;
           return false;
        }
    }

    if( _dunit.empty() || _dunit == "mv" )
        _dunit = "mV";
    if( _tunit.empty( ))
        _tunit = "ms";

    return true;
}

bool CompartmentReportBinary::_parseMapping()
{
    const uint8_t* ptr = _file.getAddress< const uint8_t >();
    if( !ptr )
        return false;

    size_t offset = _header.headerSize;

    CellInfos cells( _header.numCells );
    for( int32_t i = 0; i < _header.numCells; ++i )
    {
        CellInfo& cell = cells[i];

        cell.gid = get< int32_t >( ptr, NUMBER_OF_CELL + offset );
        cell.numCompartments = get< int32_t >( ptr,
                                              NUMBER_OF_COMPARTMENTS + offset );
        cell.mappingOffset = get< uint64_t >( ptr, MAPPING_INFO + offset );
        cell.extraMappingOffset = get< uint64_t >( ptr,
                                                  EXTRA_MAPPING_INFO + offset );
        cell.dataOffset = get< uint64_t >( ptr, DATA_INFO + offset );
        offset += SIZE_CELL_INFO_LENGTH;

        if( _header.byteswap )
            lunchbox::byteswap( cell );
    }

    _header.dataBlockOffset = cells[0].dataOffset;

    std::sort( cells.begin(), cells.end( ));
    SectionOffsets& offsetMapping = _offsets[0];
    offsetMapping.resize( cells.size( ));
    CompartmentCounts& compartmentCounts = _counts[0];
    compartmentCounts.resize( cells.size( ));

    // According to Garik Suess, all compartments of a cell in a frame are next
    // to each other, and all compartments of a section are next to each other.
    // However, the sections are not necessarily sorted by their index in the
    // frame. It could be that for cell with GID 50 while all data is
    // contiguous, the sections are out of order so compartments for section 3 6
    // 22 8 could be next to each other, while the compartments inside these
    // sections are in order.
    size_t idx = 0;
    BOOST_FOREACH( const CellInfo& info, cells )
    {
        uint16_t current = LB_UNDEFINED_UINT16;
        uint16_t previous = LB_UNDEFINED_UINT16;
        uint16_t count = 0;

        // < sectionID, < frameIndex, numCompartments > >
        typedef std::map< uint16_t,
                          std::pair< uint64_t, uint16_t > > SectionMapping;
        SectionMapping sectionsMapping;

        for( int32_t j = 0; j < info.numCompartments; ++j )
        {
            previous = current;
            const size_t pos( info.mappingOffset +
                              j * sizeof(float) * _header.mappingSize );
            float value = get< float >( ptr, pos );
            if( _header.byteswap )
                lunchbox::byteswap( value );
            current = value;

            // in case this is the start of a new section
            if( current != previous )
            {
                const uint64_t frameIndex = j + ((info.dataOffset -
                                      _header.dataBlockOffset) / sizeof(float));

                sectionsMapping.insert( std::make_pair( current,
                                              std::make_pair( frameIndex, 0 )));

                if( previous != LB_UNDEFINED_UINT16 )
                    sectionsMapping[previous].second = count;

                count = 0;
            }
            ++count;
        }
        sectionsMapping[current].second = count;

        // now convert the maps into the desired mapping format
        uint64_ts& sectionOffsets = offsetMapping[idx];
        uint16_ts& sectionCompartmentCounts = compartmentCounts[idx];
        ++idx;

        // get maximum section id
        const uint16_t maxID = sectionsMapping.rbegin()->first;
        sectionOffsets.resize( maxID + 1, LB_UNDEFINED_UINT64 );
        sectionCompartmentCounts.resize( maxID + 1, 0 );

        for( SectionMapping::const_iterator k =
             sectionsMapping.begin(); k != sectionsMapping.end(); ++k )
        {
            sectionOffsets[k->first] = k->second.first;
            sectionCompartmentCounts[k->first] = k->second.second;
        }

        _originalGIDs.insert( info.gid );
    }

    return true;
}

}
}
