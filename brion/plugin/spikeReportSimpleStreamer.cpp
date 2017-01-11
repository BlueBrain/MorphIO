/* Copyright (c) 2013-2017, EPFL/Blue Brain Project
 *                          Juan Hernando <jhernando@fi.upm.es>
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

#include "spikeReportSimpleStreamer.h"
#include "spikeReportFile.h"
#include "../pluginInitData.h"

#include <boost/filesystem/path.hpp>

namespace brion
{
namespace plugin
{

char const * const NEST_REPORT_FILE_EXT = ".gdf";
const size_t DEFAULT_LINES_PER_BATCH = 5000;

// This plugin is for testing purposes. For that reason it doesn't register
// itself.

SpikeReportSimpleStreamer::SpikeReportSimpleStreamer(
                                        const SpikeReportInitData& initData )
    : _uri( initData.getURI( ))
    , _lastTimeStamp( -1 ) // This means that nothing has been received yet
    , _lastEndTime( 0 )
{
    if( initData.getAccessMode() & MODE_WRITE )
        LBTHROW( std::runtime_error( "Writing of spike reports is not "
                                     "implemented" ));

    _readThread = boost::thread(
        boost::bind( &SpikeReportSimpleStreamer::_readLoop, this ));
}

SpikeReportSimpleStreamer::~SpikeReportSimpleStreamer()
{
    close();
    // We don't care about any other process waiting in waitUntil that may
    // have been unlocked by the call above because that's a programming error
    // (destroying the object while it is being used).
}

bool SpikeReportSimpleStreamer::handles( const SpikeReportInitData& initData )
{
    const URI& uri = initData.getURI();

    if( uri.getScheme() != "spikes" )
        return false;

    const boost::filesystem::path ext =
            boost::filesystem::path( uri.getPath() ).extension();
    return ext == NEST_REPORT_FILE_EXT;
}

std::string SpikeReportSimpleStreamer::getDescription()
{
    return std::string(
               "Spike stream test reports: [spikes://]/path/to/report" ) +
           NEST_REPORT_FILE_EXT;
}

const URI& SpikeReportSimpleStreamer::getURI() const
{
    return _uri;
}

float SpikeReportSimpleStreamer::getStartTime() const
{
    if ( _spikes.empty( ))
        return UNDEFINED_TIMESTAMP;

    return _spikes.begin()->first;
}

float SpikeReportSimpleStreamer::getEndTime() const
{
    if( _spikes.empty( ))
        return UNDEFINED_TIMESTAMP;

    return _spikes.rbegin()->first;
}

const Spikes& SpikeReportSimpleStreamer::getSpikes() const
{
    return _spikes;
}

bool SpikeReportSimpleStreamer::waitUntil( const float timeStamp,
                                           const uint32_t timeout )
{
    // Wait until a timeStamp immediately after timeStamp arrives (we use
    // nextafterf because monitor doesn't have a timedWaitGT method).
    _lastTimeStamp.timedWaitGT( timeStamp, timeout );

    float lastTimeStamp;

    {
        boost::mutex::scoped_lock lock( _mutex );

        if (!_incoming.empty())
            lastTimeStamp = _incoming.rbegin()->first;
        else
            lastTimeStamp = _lastTimeStamp.get();

        // Copying the spikes from _incoming to _spikes.
        const Spikes::iterator last = _incoming.upper_bound( timeStamp );
        _spikes.insert( _incoming.begin(), last );
        if( !_spikes.empty( ))
            _lastEndTime = _spikes.rbegin()->first;
        // And clearing the range [begin, last) from _incoming
        _incoming.erase( _incoming.begin(), last );
    }

    // _lastTimestamp can contain +inf if the stream source has been closed.
    // That value is used to make sure that this thread will unlock.
    if( _lastTimeStamp == std::numeric_limits< float >::infinity( ))
    {
        // It is safe to access _incoming after this point without a
        // mutex because the read loop is not going to touch it.
        return !_incoming.empty();
        // If some spikes have been left in incoming the next call to
        // waitUntil will transfer them.
    }

    // Reached this point, the operation can be considered successful only
    // if _incoming has some spikes left. The condition below is equivalent.
    return lastTimeStamp > timeStamp;
}

float SpikeReportSimpleStreamer::getNextSpikeTime()
{
    boost::mutex::scoped_lock lock( _mutex );

    if( _incoming.empty( ))
    {
        // The end of the stream has been reached and no spikes need to
        // be moved from incoming to the public container.
        if( _lastTimeStamp ==  std::numeric_limits< float >::infinity( ))
            return UNDEFINED_TIMESTAMP;
        // This works either for the case in which nothing has been read
        // yet and when incoming is empty and we have to return the spike
        // time that guarantees that waitUntil will make progress.
        return _lastEndTime;
    }
    return _incoming.begin()->first;
}

float SpikeReportSimpleStreamer::getLatestSpikeTime()
{
    const float lastTimeStamp = _lastTimeStamp.get();
    if( lastTimeStamp == -1 )
        return UNDEFINED_TIMESTAMP;
    if( lastTimeStamp ==  std::numeric_limits< float >::infinity( ))
    {
        // No mutex needed because _readLoop won't write to _incoming after
        // setting lastTimeStamp to inf.
        if( _incoming.empty( ))
            return _lastEndTime;
        return _incoming.rbegin()->first;
    }
    return lastTimeStamp;
}

void SpikeReportSimpleStreamer::clear( const float startTime,
                                       const float endTime )
{
    if( endTime < startTime )
        return;
    _spikes.erase( _spikes.lower_bound( startTime ),
                   _spikes.upper_bound( endTime ));
}

void SpikeReportSimpleStreamer::close()
{
    _readThread.interrupt();
    _readThread.join();
    // When _readLoop is interrupted before reaching EOF, it never carries
    // out this assigment.
    _lastTimeStamp = std::numeric_limits< float >::infinity();
}

void SpikeReportSimpleStreamer::_readLoop()
{
    SpikeReportFile reader( _uri.getPath(), NEST_SPIKE_REPORT, MODE_READ );

    bool eof = false;
    while( !eof )
    {
        try
        {
            boost::mutex::scoped_lock lock( _mutex );
            eof = reader.fillReportMap( _incoming, DEFAULT_LINES_PER_BATCH );
            if( !_incoming.empty() )
                _lastTimeStamp = _incoming.rbegin()->first;
        }
        catch( std::runtime_error& )
        {
            _lastTimeStamp = std::numeric_limits< float >::infinity( );
            return;
        }
        boost::this_thread::interruption_point();
    }
    _lastTimeStamp = std::numeric_limits< float >::infinity();
}

}
}
