/* Copyright (c) 2015-2017, EPFL/Blue Brain Project
 *                          Stefan Eilemann <stefan.eilemann@epfl.ch>
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

#include <brion/brion.h>
#include <lunchbox/clock.h>
#include <lunchbox/file.h>
#include <lunchbox/log.h>
#include <lunchbox/sleep.h>
#include <lunchbox/string.h>

#define STREAM_READ_TIMEOUT_MS  500
#define STREAM_SEND_DELAY_MS   1000
#define STREAM_SEND_FREQ_MS     500
#define STREAM_FRAME_LENGTH_MS   10

void printSentFrame( const unsigned int index, const size_t count )
{
    LBINFO << "Sent frame " << index << ": " << index * STREAM_FRAME_LENGTH_MS
           << "-" << (index+1) * STREAM_FRAME_LENGTH_MS << " [ms], "
           << count << " spikes" << std::endl;
}

int main( int argc, char* argv[] )
{
    if( argc != 3 )
    {
        const auto uriHelp =
            lunchbox::string::prepend( brion::SpikeReport::getDescriptions(),
                                       "    " );
        std::cout << "Usage: " << lunchbox::getFilename( argv[0] )
                  << " <inURI> <outURI>" << std::endl
                  << "  Supported input and output URIs:" << std::endl
                  << uriHelp << std::endl;
        return EXIT_FAILURE;
    }

    lunchbox::Clock clock;
    brion::SpikeReport in( brion::URI( argv[1] ), brion::MODE_READ );
    const float readTime = clock.resetTimef();

    brion::SpikeReport out( brion::URI( argv[2] ), brion::MODE_WRITE );

    if( in.getReadMode() == brion::SpikeReport::STREAM )
    {
        // Stream-to-File conversion
        while( in.getNextSpikeTime() != brion::UNDEFINED_TIMESTAMP )
        {
            in.waitUntil( brion::UNDEFINED_TIMESTAMP, STREAM_READ_TIMEOUT_MS );
            out.writeSpikes( in.getSpikes( ));
            in.clear( in.getStartTime( ), in.getEndTime( ));
        }
    }
    else if( out.getReadMode() == brion::SpikeReport::STREAM )
    {
        // File-to-Stream conversion

        // leave time for a Zeq connection to establish
        lunchbox::sleep( STREAM_SEND_DELAY_MS );

        brion::Spikes outSpikes;
        unsigned int frameIndex = 0;

        const brion::Spikes& inSpikes = in.getSpikes();
        for( brion::Spikes::const_iterator spikeIt = inSpikes.begin();
             spikeIt != inSpikes.end(); ++spikeIt )
        {
            if( spikeIt->first >= (frameIndex+1) * STREAM_FRAME_LENGTH_MS )
            {
                out.writeSpikes( outSpikes );
                printSentFrame( frameIndex++, outSpikes.size( ));
                outSpikes.clear();
                lunchbox::sleep( STREAM_SEND_FREQ_MS );
            }
            outSpikes.insert( *spikeIt );
        }
        out.writeSpikes( outSpikes );
        printSentFrame( frameIndex, outSpikes.size( ));
    }
    else
    {
        // File-to-File conversion
        out.writeSpikes( in.getSpikes( ));
    }

    out.close();
    const float writeTime = clock.resetTimef();

    LBINFO << "Converted " << argv[1] << " => " << argv[2] << " in " << readTime
           << " + " << writeTime << " ms" << std::endl;
    return EXIT_SUCCESS;
}
