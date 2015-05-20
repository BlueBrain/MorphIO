/* Copyright (c) 2015, EPFL/Blue Brain Project
 *                     Stefan Eilemann <stefan.eilemann@epfl.ch>
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

int main( int argc, char* argv[] )
{
    if( argc != 3 )
    {
        LBERROR << "Usage: " << argv[0] <<  " <inURI> <outURI>" << std::endl;
        return EXIT_FAILURE;
    }

    lunchbox::Clock clock;
    const brion::SpikeReport in( brion::URI( argv[1] ), brion::MODE_READ );
    const float readTime = clock.resetTimef();

    brion::SpikeReport out( brion::URI( argv[2] ), brion::MODE_WRITE );
    out.writeSpikes( in.getSpikes( ));
    out.close();
    const float writeTime = clock.resetTimef();

    LBINFO << "Converted " << argv[1] << " => " << argv[2] << " in " << readTime
           << " + " << writeTime << " ms" << std::endl;
    return EXIT_SUCCESS;
}
