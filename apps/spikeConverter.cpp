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
#include <lunchbox/term.h>
#ifdef BRION_USE_BBPTESTDATA
#  include <BBP/TestDatasets.h>
#endif

#define STREAM_READ_TIMEOUT_MS 500
#define STREAM_SEND_DELAY_MS 1000
#define STREAM_SEND_FREQ_MS 500
#define STREAM_FRAME_LENGTH_MS 10

#include <boost/program_options.hpp>
#include <boost/progress.hpp>

namespace po = boost::program_options;

int main( int argc, char* argv[] )
{
    const std::string help = lunchbox::getFilename( std::string( argv[0] ));
    const std::string uriHelp = std::string( "Output report URI\n" ) +
        "  Supported input and output URIs:\n" +
        lunchbox::string::prepend( brion::SpikeReport::getDescriptions(),
                                   "    " );

    po::options_description options( help.c_str(),
                                     lunchbox::term::getSize().first );
    options.add_options()
        ( "help,h", "Produce help message" )
        ( "version,v", "Show program name/version banner and exit" )
#ifdef BRION_USE_BBPTESTDATA
        ( "input,i", po::value< std::string >()->default_value(
            std::string( BBP_TESTDATA ) +
            "/circuitBuilding_1000neurons/Neurodamus_output/out.dat" ),
          "Input report URI" )
#else
        ( "input,i", po::value< std::string >()->required(), "Input report URI")
#endif
        ( "output,o", po::value< std::string >()->default_value( "out.spikes" ),
          uriHelp.c_str( ));
    
    po::positional_options_description positional;
    positional.add("input", 1);
    positional.add("output", 2);    
    
    po::variables_map vm;
    try
    {
        po::store( po::command_line_parser( argc, argv ).
                   options( options ).positional( positional ).run(), vm );       
        po::notify( vm );
    }
    catch( const po::error& e )
    {
        std::cerr << "Command line parse error: " << e.what() << std::endl
                  << options << std::endl;
        return EXIT_FAILURE;
    }

    if( vm.count( "help" ))
    {
        std::cout << options << std::endl;
        return EXIT_SUCCESS;
    }

    if( vm.count( "version" ))
    {
        std::cout << "Brion spike report converter "
                  << brion::Version::getString() << std::endl;
        return EXIT_SUCCESS;
    }

    try
    {
        lunchbox::Clock clock;

        float readTime = 0.f;
        brion::SpikeReport in( brion::URI( vm["input"].as< std::string >( )),
                               brion::MODE_READ );
        readTime += clock.resetTimef();

        float writeTime = 0.f;
        brion::SpikeReport out( brion::URI( vm["output"].as< std::string >( )),
                                brion::MODE_WRITE );
        writeTime += clock.resetTimef();

        const float step = 10.f; //ms, arbitrary value
        while( in.getState() == brion::SpikeReport::State::ok )
        {
            const auto spikes = in.readUntil( in.getCurrentTime() + step ).get();
            readTime += clock.resetTimef();

            out.write( spikes );
            writeTime += clock.resetTimef();
        }

        std::cout << "Converted " << vm["input"].as< std::string >() << " => "
                  << vm["output"].as< std::string >() << " in " << readTime
                  << " + " << writeTime << " ms" << std::endl;
    }
    catch ( const std::exception& exception )
    {
        LBINFO << "Failed to convert spikes: " << exception.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
