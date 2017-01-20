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

#define STREAM_READ_TIMEOUT_MS 500
#define STREAM_SEND_DELAY_MS 1000
#define STREAM_SEND_FREQ_MS 500
#define STREAM_FRAME_LENGTH_MS 10

void printSentFrame( const unsigned int index, const size_t count )
{
    LBINFO << "Sent frame " << index << ": " << index * STREAM_FRAME_LENGTH_MS << "-"
           << ( index + 1 ) * STREAM_FRAME_LENGTH_MS << " [ms], " << count << " spikes"
           << std::endl;
}

int main( int argc, char* argv[] )
{
    if ( argc != 3 )
    {
        const auto uriHelp =
            lunchbox::string::prepend( brion::SpikeReport::getDescriptions(),
                                       "    " );
        std::cout << "Usage: " << lunchbox::getFilename( argv[0] )
                  << " <inURI> <outURI>" 
                  << "  Supported input and output URIs:" << std::endl
                  << uriHelp << std::endl;
        return EXIT_FAILURE;
    }

    lunchbox::Clock clock;
    const float readTime = clock.resetTimef();

    try
    {
        brion::SpikeReport in( brion::URI( argv[1] ), brion::MODE_READ );
        brion::SpikeReport out( brion::URI( argv[2] ), brion::MODE_WRITE );

        const float step = 0.1; // arbitrary value

        float t = std::max( step, in.getCurrentTime() );
        std::vector< brion::Spike > spikes;
        while ( in.getState() == brion::SpikeReport::State::ok )
        {
            spikes = in.read( t ).get();
            out.write( spikes );
            t = std::max( t + step, in.getCurrentTime() );
        }
    }
    catch ( const std::exception& exception )
    {
        LBINFO << "Failed to conver spikes : " << exception.what() << std::endl;
        return EXIT_FAILURE;
    }

    const float writeTime = clock.resetTimef();

    LBINFO << "Converted " << argv[1] << " => " << argv[2] << " in " << readTime << " + "
           << writeTime << " ms" << std::endl;

    return EXIT_SUCCESS;
}
