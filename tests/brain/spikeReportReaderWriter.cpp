
/* Copyright (c) 2006-2015, Juan Hernando <jhernando@fi.upm.es>
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

#include <brain/brain.h>
#include <brain/spikeReportReader.h>
#include <brain/spikeReportWriter.h>

#include <BBP/TestDatasets.h>

#include <lunchbox/sleep.h>
#include <lunchbox/uint128_t.h>

#include <brion/types.h>

#define BOOST_TEST_MODULE SpikeReportReader
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/thread/thread.hpp>

#define BLURON_SPIKE_REPORT_FILE  "local/simulations/may17_2011/Control/out.dat"

#define BLURON_SPIKES_START_TIME 0.15f
#define BLURON_SPIKES_END_TIME 9.975f
#define BLURON_SPIKES_COUNT 274

#define BLURON_FIRST_SPIKE_TIME BLURON_SPIKES_START_TIME
#define BLURON_FIRST_SPIKE_GID 290
#define BLURON_LAST_SPIKE_TIME BLURON_SPIKES_END_TIME
#define BLURON_LAST_SPIKE_GID 353

#define NEST_SPIKE_REPORT_FILE "NESTSpikeData/spike_detector-65537-00.gdf"

#define NEST_SPIKES_START_TIME 1.8f
#define NEST_SPIKES_END_TIME 98.8f
#define NEST_SPIKES_COUNT 96256

#define NEST_FIRST_SPIKE_TIME NEST_SPIKES_START_TIME
#define NEST_FIRST_SPIKE_GID 33872

#define NEST_LAST_SPIKE_TIME NEST_SPIKES_END_TIME
#define NEST_LAST_SPIKE_GID 47760

struct TmpFile
{
    const std::string name;

    explicit TmpFile( const std::string& suffix = std::string( ))
        : name("/tmp/" + lunchbox::make_UUID().getString() + suffix)
    {
    }

    ~TmpFile()
    {
        if( boost::filesystem::exists( name ))
            boost::filesystem::remove( name );
    }
};

namespace std
{
    std::ostream& operator<<( std::ostream &os, const brion::Spike& spike )
    {
        return os << spike.first << ", " << spike.second;
    }
}

BOOST_AUTO_TEST_CASE( test_invalid_report )
{
    BOOST_CHECK_THROW( brain::SpikeReportReader report0( brion::URI( "./bla" )),
                       std::runtime_error );

    boost::filesystem::path path( BBP_TESTDATA );
    path /= "local/README";
    BOOST_CHECK_THROW( brain::SpikeReportReader( brion::URI( path.string( ))),
                       std::runtime_error );
}

BOOST_AUTO_TEST_CASE( test_simple_load_static )
{
    boost::filesystem::path path( BBP_TESTDATA );
    path /= BLURON_SPIKE_REPORT_FILE;

    brain::SpikeReportReader reader( brion::URI( path.string( )));
}



BOOST_AUTO_TEST_CASE( test_simple_read )
{
    boost::filesystem::path path( BBP_TESTDATA );
    path /= BLURON_SPIKE_REPORT_FILE;

    brain::SpikeReportReader reader( brion::URI( path.string( )));
    const brion::Spikes& spikes = reader.getSpikes(0,brion::UNDEFINED_TIMESTAMP);

    BOOST_REQUIRE_EQUAL( spikes.size(), BLURON_SPIKES_COUNT );

    BOOST_CHECK_EQUAL( spikes.begin()->first, BLURON_SPIKES_START_TIME );
    BOOST_CHECK_EQUAL( spikes.begin()->second, BLURON_FIRST_SPIKE_GID );

    BOOST_CHECK_EQUAL( ( --spikes.end( ))->first, BLURON_LAST_SPIKE_TIME );
    BOOST_CHECK_EQUAL( ( --spikes.end( ))->second, BLURON_LAST_SPIKE_GID );
}


BOOST_AUTO_TEST_CASE( test_closed_window )
{
    boost::filesystem::path path( BBP_TESTDATA );
    path /= BLURON_SPIKE_REPORT_FILE;
    brain::SpikeReportReader reader( brion::URI( path.string( )));
    BOOST_CHECK_THROW(reader.getSpikes( 2.5f, 2.5f ),std::logic_error);
}

BOOST_AUTO_TEST_CASE( test_out_of_window )
{
    boost::filesystem::path path( BBP_TESTDATA );
    path /= BLURON_SPIKE_REPORT_FILE;
    brain::SpikeReportReader reader( brion::URI( path.string( )));
    const brion::Spikes& spikes = reader.getSpikes(0,brion::UNDEFINED_TIMESTAMP);

    const float start = spikes.back().first + 1;

    BOOST_CHECK_THROW( reader.getSpikes(start, start + 1 ), std::logic_error );
}

BOOST_AUTO_TEST_CASE( test_simple_stream_read )
{
    boost::filesystem::path path( BBP_TESTDATA );
    path /= NEST_SPIKE_REPORT_FILE;

    brain::SpikeReportReader reader( brion::URI( path.string( )));

    const brion::Spikes& spikes = reader.getSpikes(0,brion::UNDEFINED_TIMESTAMP);

    BOOST_REQUIRE_EQUAL( spikes.size(), NEST_SPIKES_COUNT );

    BOOST_CHECK_EQUAL( spikes.begin()->first, NEST_FIRST_SPIKE_TIME );
    BOOST_CHECK_EQUAL( spikes.begin()->second, NEST_FIRST_SPIKE_GID );

    BOOST_CHECK_EQUAL( ( --spikes.end( ))->first, NEST_LAST_SPIKE_TIME );
    BOOST_CHECK_EQUAL( ( --spikes.end( ))->second, NEST_LAST_SPIKE_GID );
}

BOOST_AUTO_TEST_CASE( test_moving_window )
{
    boost::filesystem::path path( BBP_TESTDATA );
    path /= NEST_SPIKE_REPORT_FILE;

    brain::SpikeReportReader reader( brion::URI( path.string( )));

    float start = 0;
    while( !reader.hasEnded( ))
    {
        const brion::Spikes& spikes = reader.getSpikes(start, start + 1);
        if( !spikes.empty( ))
        {

            BOOST_CHECK( spikes.begin()->first >= start );
            BOOST_CHECK( ( --spikes.end( ))->first >= start );
        }
        start += 1;
    }

    const brion::Spikes& spikes = reader.getSpikes(0,brion::UNDEFINED_TIMESTAMP);

    BOOST_REQUIRE_EQUAL( spikes.size(), NEST_SPIKES_COUNT );


    BOOST_CHECK_EQUAL( spikes.begin()->first, NEST_FIRST_SPIKE_TIME );
    BOOST_CHECK_EQUAL( spikes.begin()->second, NEST_FIRST_SPIKE_GID );

    BOOST_CHECK_EQUAL( ( --spikes.end( ))->first, NEST_LAST_SPIKE_TIME );
    BOOST_CHECK_EQUAL( ( --spikes.end( ))->second, NEST_LAST_SPIKE_GID );
}

BOOST_AUTO_TEST_CASE( TestSpikes_nest_spikes_read_write )
{
    boost::filesystem::path path( BBP_TESTDATA );
    path /= BLURON_SPIKE_REPORT_FILE;

    brain::SpikeReportReader reader( brion::URI( path.string( )));
    const brion::Spikes& readSpikes = reader.getSpikes(0,brion::UNDEFINED_TIMESTAMP);

    TmpFile file( ".gdf" );

    brain::SpikeReportWriter writer( brion::URI( file.name ));
    writer.writeSpikes( readSpikes );
    writer.close();

    brain::SpikeReportReader reReader( brion::URI( file.name ));
    const brion::Spikes& reReadSpikes = reReader.getSpikes(0,brion::UNDEFINED_TIMESTAMP);

    BOOST_CHECK_EQUAL_COLLECTIONS(
        readSpikes.begin(), readSpikes.end(),
        reReadSpikes.begin(), reReadSpikes.end( ));
}

BOOST_AUTO_TEST_CASE( TestSpikes_bluron_spikes_read_write )
{
    boost::filesystem::path path( BBP_TESTDATA );
    path /= BLURON_SPIKE_REPORT_FILE;

    brain::SpikeReportReader reader( brion::URI( path.string( )));
    const brion::Spikes& readSpikes = reader.getSpikes(0,brion::UNDEFINED_TIMESTAMP);

    TmpFile file( ".dat" );

    brain::SpikeReportWriter writer( brion::URI( file.name ));
    writer.writeSpikes( readSpikes );
    writer.close();

    brain::SpikeReportReader reReader( brion::URI( file.name ));
    const brion::Spikes& reReadSpikes = reReader.getSpikes(0,brion::UNDEFINED_TIMESTAMP);

    BOOST_CHECK_EQUAL_COLLECTIONS( readSpikes.begin(), readSpikes.end(),
                                   reReadSpikes.begin(), reReadSpikes.end( ));
}
