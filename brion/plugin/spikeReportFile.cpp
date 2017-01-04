/* Copyright (c) 2013-2017, EPFL/Blue Brain Project
 *                          Raphael Dumusc <raphael.dumusc@epfl.ch>
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

#include "spikeReportFile.h"
#include "../detail/skipWhiteSpace.h"

#include <lunchbox/log.h>
#include <lunchbox/stdExt.h>
#include <boost/algorithm/string.hpp>
#include <boost/bind.hpp>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/regex.hpp>
#include <boost/version.hpp>
#include <fstream>

namespace brion
{
namespace plugin
{

namespace
{

template< typename T >
static std::string lexical_cast( const T& t )
{
    return boost::lexical_cast< std::string >( t );
}

// According to http://www.boost.org/doc/libs/1_56_0/doc/html/boost_lexical_cast/performance.html
// sscanf if faster than stringstream, and lexical_cast is faster than sscanf
// However, it seems that in combination with the OpenMP parallel for, the
// code with sscanf is performing slightly better than lexical_cast.

bool parseBluronLine( const std::string& buffer,
                      SpikeReportFile::Spike &spike )
{
    return sscanf( buffer.data( ), "%20f%20ud",
                   &spike.first, &spike.second ) == 2;
}

bool parseNESTLine( const std::string& buffer,
                    SpikeReportFile::Spike &spike )
{
    return sscanf (buffer.data( ), "%20d%20f",
                   &spike.second, &spike.first ) == 2;
}


void writeBluronLine( std::fstream& file,
                      const SpikeReportFile::Spike &spike  )
{
    file << spike.first << " " << spike.second << std::endl;
}

void writeNESTLine( std::fstream& file,
                      const SpikeReportFile::Spike &spike  )
{
    file << spike.second << " " << spike.first << std::endl;
}

}

SpikeReportFile::SpikeReportFile( const std::string& filename,
                                  const SpikeReportType rt,
                                  const int accessMode )
    : _filename( filename )
    , _reportType( rt )
{
    if( filename.empty( ))
    {
        LBTHROW( std::runtime_error( "Error opening spike times file:'" +
                                     _filename + "'." ));
    }

    std::ios_base::openmode fileMode = std::ios_base::binary;
    if( accessMode & MODE_WRITE )
    {
        if(( accessMode & MODE_OVERWRITE ) != MODE_OVERWRITE
                && boost::filesystem::exists( _filename ))
        {
            LBTHROW( std::runtime_error( "Cannot overwrite existing file:'" +
                                         _filename + "'." ));
        }

        fileMode |= std::ios_base::out;
        switch( _reportType )
        {
        case BLURON_SPIKE_REPORT:
            _spikeWriteFunction = boost::bind( &writeBluronLine, _1, _2 );
            break;
        case NEST_SPIKE_REPORT:
            _spikeWriteFunction = boost::bind( &writeNESTLine, _1, _2 );
            break;
        default:
            LBTHROW( std::runtime_error( "Report type not implemented" ));
        }
    }

    if( accessMode & MODE_READ )
    {
        fileMode |= std::ios_base::in;
        switch( _reportType )
        {
        case BLURON_SPIKE_REPORT:
            _spikeParseFunction = boost::bind( &parseBluronLine, _1, _2 );
            break;
        case NEST_SPIKE_REPORT:
            _spikeParseFunction = boost::bind( &parseNESTLine, _1, _2 );
            break;
        default:
            LBTHROW( std::runtime_error( "Report type not implemented" ));
        }
    }

    _file.reset( new std::fstream( _filename.c_str(), fileMode ));
}

SpikeReportFile::~SpikeReportFile()
{
}

bool SpikeReportFile::fillReportMap( Spikes& spikes, const size_t maxLines )
{
    if( _spikeWriteFunction )
        LBTHROW( std::runtime_error( "File is not opened for reading " +
                                     _filename ));

    size_t lineNumber = 0;
    *_file >> detail::SkipWhiteSpace( lineNumber );

    std::vector< Spike > spikeArray;

#ifdef BRION_USE_OPENMP
    // The batch size is divisible by all reasonable core counts between 1
    // and 50.
    const size_t batchSize = 554400;
    size_t remaining = maxLines;

    while( remaining != 0 && !(*_file).fail( ))
    {
        const size_t currentBatch =
            maxLines == std::numeric_limits< size_t >::max() ?
            batchSize : std::min( remaining, batchSize );

        // Reading a batch of strings to parse in parallel
        Strings lines( currentBatch );
        size_t linesRead = 0;

        while( linesRead != lines.size() &&
               !std::getline( *_file, lines[linesRead] ).fail( ))
        {
            // This check skips comments
            const char c = lines[linesRead][0];
            if( c != '/' && c != '#' )
                ++linesRead;
            size_t skipCount = 0;
            *_file >> detail::SkipWhiteSpace( skipCount );
        }
        remaining -= std::min( remaining, linesRead );

        const size_t first = spikeArray.size( );
        spikeArray.resize( first + linesRead );

        // Parsing strings
        #pragma omp parallel for
        for( int64_t i = 0; i < int64_t(linesRead); ++i )
        {
            const std::string& line = lines[i];
            if( !_spikeParseFunction( line.c_str( ), spikeArray[first + i] ))
            {
                // Getting the line number right is not possible unless the
                // line numbers are stored along with the strings.
                // Instead of telling the line number, the full line will be
                // printed. The error should be easy to spot in the file as
                // it will be found at the first occurence in the file.
                LBERROR << "Error reading spike times file: " << _filename
                        << ", line: " << lines[i] << std::endl;
            }
        }
    }
#else
    std::string line;
    size_t remaining = maxLines;
    while( remaining != 0 && !std::getline( *_file, line ).fail( ))
    {
        ++lineNumber;
        if ( line[0] == '/' || line[0] == '#' )
             continue;

        --remaining;
        Spike spike;
        if( !_spikeParseFunction( line.c_str( ), spike ))
        {
            LBTHROW( std::runtime_error( "Reading spike times file: " +
                                         _filename + ", line " +
                                         lexical_cast( lineNumber )));
        }
        spikeArray.push_back( spike );

        *_file >> detail::SkipWhiteSpace( lineNumber );
    }
#endif

    // Inserting a range of sorted elements is faster that inserting one
    // by one.
    std::sort( spikeArray.begin(), spikeArray.end( ));
    spikes.insert( spikeArray.begin(), spikeArray.end( ));

    if( _file->fail( ) && !_file->eof( ))
        LBTHROW( std::runtime_error( "IO error reading spike times file: " +
                                     _filename ));
    return _file->eof();
}

void SpikeReportFile::writeReportMap( const Spikes& spikes )
{
    if( !_spikeWriteFunction )
        LBTHROW( std::runtime_error( "File is not opened for writing " +
                                     _filename ));

    for( Spikes::const_iterator it = spikes.begin(); it != spikes.end(); ++it )
        _spikeWriteFunction( *_file, *it );
}

void SpikeReportFile::close()
{
    if( _file )
        _file->close();
}

}
}
