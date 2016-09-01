/* Copyright (c) 2014-2015, EPFL/Blue Brain Project
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
 * This file is part of Brion <https://github.com/BlueBrain/Brion>
 */

#include <brion/brion.h>
#include <BBP/TestDatasets.h>
#include <lunchbox/uint128_t.h>

#define BOOST_TEST_MODULE SpikeReport
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/test/unit_test.hpp>

#define BLURON_SPIKE_REPORT_FILE  "local/simulations/may17_2011/Control/out.dat"

#define BLURON_SPIKES_START_TIME 0.15f
#define BLURON_SPIKES_END_TIME 9.975f

#define NEST_SPIKES_START_TIME 1.8f
#define NEST_SPIKES_END_TIME 98.9f

#define BLURON_SPIKES_COUNT 274
#define NEST_SPIKES_COUNT 1540096

#define BLURON_FIRST_SPIKE_TIME BLURON_SPIKES_START_TIME
#define BLURON_FIRST_SPIKE_GID 290

#define NEST_FIRST_SPIKE_TIME NEST_SPIKES_START_TIME
#define NEST_FIRST_SPIKE_GID 32826
#define NEST_FIRST_SPIKE_GID_COUNT 1114

#define BLURON_LAST_SPIKE_TIME BLURON_SPIKES_END_TIME
#define BLURON_LAST_SPIKE_GID 353

#define NEST_LAST_SPIKE_TIME NEST_SPIKES_END_TIME
#define NEST_LAST_SPIKE_GID 40596

class TemporaryData
{
public:
    brion::Spikes spikes;
    const std::string tmpFileName;

    explicit TemporaryData( const std::string& reportType )
        : tmpFileName( "/tmp/" + lunchbox::make_UUID().getString() + "." +
                       reportType )
    {
        spikes.insert( brion::Spike( 0.1f, 20 ));
        spikes.insert( brion::Spike( 0.2f, 22 ));
        spikes.insert( brion::Spike( 0.2f, 23 ));
        spikes.insert( brion::Spike( 0.3f, 24 ));
        spikes.insert( brion::Spike( 0.4f, 25 ));
    };

    ~TemporaryData()
    {
        if( boost::filesystem::exists( tmpFileName ))
            boost::filesystem::remove( tmpFileName );
    }
};

BOOST_AUTO_TEST_CASE( test_bluron_invalid_open )
{
    BOOST_CHECK_THROW( brion::SpikeReport report0( brion::URI( "./bla" ),
                                                   brion::MODE_READ ),
                       std::runtime_error );

    boost::filesystem::path path( BBP_TESTDATA );
    path /= "local/README";
    BOOST_CHECK_THROW( brion::SpikeReport report1( brion::URI( path.string( )),
                                                   brion::MODE_READ ),
                       std::runtime_error );

    path = BBP_TESTDATA;
    path /= "local/simulations/may17_2011/Control/voltage.h5";
    BOOST_CHECK_THROW( brion::SpikeReport report2( brion::URI( path.string( )),
                                                   brion::MODE_READ),
                       std::runtime_error );
}

BOOST_AUTO_TEST_CASE( test_nest_invalid_open )
{
    BOOST_CHECK_THROW( brion::SpikeReport report0( brion::URI( "./bla" ),
                                                   brion::MODE_READ ),
                       std::runtime_error );

    boost::filesystem::path path( BBP_TESTDATA );
    path /= "local/README";
    BOOST_CHECK_THROW( brion::SpikeReport report1( brion::URI( path.string( )),
                                                   brion::MODE_READ ),
                       std::runtime_error );

    path = BBP_TESTDATA;
    path /= "local/simulations/may17_2011/Control/voltage.h5";
    BOOST_CHECK_THROW( brion::SpikeReport report2( brion::URI( path.string( )),
                                                   brion::MODE_READ ),
                       std::runtime_error );
}

BOOST_AUTO_TEST_CASE( test_bluron_invalid_report_information )
{
    boost::filesystem::path path( BBP_TESTDATA );
    path /= BLURON_SPIKE_REPORT_FILE;

    BOOST_CHECK_THROW( const brion::SpikeReport report(
                           brion::URI( path.string() + ";" + path.string( )),
                           brion::MODE_READ ),
                       std::runtime_error );
}

BOOST_AUTO_TEST_CASE( test_bluron_verify_loaded_data )
{
    boost::filesystem::path path( BBP_TESTDATA );
    path /= BLURON_SPIKE_REPORT_FILE;

    const brion::SpikeReport report( brion::URI( path.string( )),
                                     brion::MODE_READ );
    const brion::Spikes& spikes = report.getSpikes();

    BOOST_REQUIRE_EQUAL( report.getReadMode(), brion::SpikeReport::STATIC );

    BOOST_REQUIRE_EQUAL( spikes.size(), BLURON_SPIKES_COUNT );

    BOOST_CHECK_EQUAL( report.getStartTime(), BLURON_SPIKES_START_TIME );
    BOOST_CHECK_EQUAL( report.getEndTime(), BLURON_SPIKES_END_TIME );

    BOOST_CHECK_EQUAL( spikes.begin()->first, BLURON_SPIKES_START_TIME );
    BOOST_CHECK_EQUAL( spikes.begin()->second, BLURON_FIRST_SPIKE_GID );

    BOOST_CHECK_EQUAL( spikes.rbegin()->first, BLURON_LAST_SPIKE_TIME );
    BOOST_CHECK_EQUAL( spikes.rbegin()->second, BLURON_LAST_SPIKE_GID );
}

BOOST_AUTO_TEST_CASE( test_bluron_invoke_invalid_method )
{
    boost::filesystem::path path( BBP_TESTDATA );
    path /= BLURON_SPIKE_REPORT_FILE;

    brion::SpikeReport report( brion::URI( path.string( )),
                               brion::MODE_READ );
    brion::Spikes spikes;
    BOOST_CHECK_THROW( report.writeSpikes( spikes ),
                       std::runtime_error );
}

BOOST_AUTO_TEST_CASE( test_nest_invoke_invalid_method )
{
    boost::filesystem::path path( BBP_TESTDATA );
    const std::string& files = "NESTSpikeData/spike_detector-65537-*.gdf";

    brion::SpikeReport report(
                       brion::URI(( path / files ).string( )),
                       brion::MODE_READ );
    brion::Spikes spikes;
    BOOST_CHECK_THROW( report.writeSpikes( spikes ),
                       std::runtime_error );
}

BOOST_AUTO_TEST_CASE( test_nest_verify_loaded_data )
{
    boost::filesystem::path path( BBP_TESTDATA );
    const std::string& files = "NESTSpikeData/spike_detector-65537-*.gdf";

    const brion::SpikeReport report( brion::URI(( path / files ).string( )),
                                     brion::MODE_READ );
    BOOST_REQUIRE_EQUAL( report.getReadMode(), brion::SpikeReport::STATIC );

    const brion::Spikes& spikes = report.getSpikes();

    BOOST_REQUIRE_EQUAL( spikes.size(), NEST_SPIKES_COUNT );

    BOOST_CHECK_EQUAL( report.getStartTime(), NEST_SPIKES_START_TIME );
    BOOST_CHECK_EQUAL( report.getEndTime(), NEST_SPIKES_END_TIME );

    // The spikes are ordered by time but not by GIDs.
    // Extract the sorted set of GIDs corresponding to the first spike time.
    std::set<int> gids;
    brion::Spikes::const_iterator it = spikes.begin();
    while( it != spikes.end() && it->first == NEST_SPIKES_START_TIME )
    {
        gids.insert(it->second);
        ++it;
    }
    BOOST_REQUIRE_EQUAL( gids.size(), NEST_FIRST_SPIKE_GID_COUNT );
    BOOST_CHECK_EQUAL( *gids.begin(), NEST_FIRST_SPIKE_GID );

    BOOST_CHECK_EQUAL( spikes.rbegin()->first, NEST_LAST_SPIKE_TIME );
    BOOST_CHECK_EQUAL( spikes.rbegin()->second, NEST_LAST_SPIKE_GID );
}

BOOST_AUTO_TEST_CASE( test_write_nest_data )
{
    TemporaryData data( "gdf" );

    brion::SpikeReport reportWrite( brion::URI( data.tmpFileName ),
                                    brion::MODE_WRITE );

    reportWrite.writeSpikes( data.spikes );
    reportWrite.close();

    const brion::SpikeReport reportRead( brion::URI( data.tmpFileName ),
                                           brion::MODE_READ );

    const brion::Spikes& readSpikes = reportRead.getSpikes();

    BOOST_CHECK_EQUAL_COLLECTIONS( data.spikes.begin(), data.spikes.end(),
                                   readSpikes.begin(), readSpikes.end( ));
}

BOOST_AUTO_TEST_CASE( test_write_bluron_data )
{
    TemporaryData data( "dat" );

    brion::SpikeReport reportWrite( brion::URI( data.tmpFileName ),
                                    brion::MODE_WRITE );
    reportWrite.writeSpikes( data.spikes );
    reportWrite.close();

    const brion::SpikeReport reportRead( brion::URI( data.tmpFileName ),
                                         brion::MODE_READ );

    const brion::Spikes& readSpikes = reportRead.getSpikes();

    BOOST_CHECK_EQUAL_COLLECTIONS( data.spikes.begin(), data.spikes.end(),
                                   readSpikes.begin(), readSpikes.end( ));
}

BOOST_AUTO_TEST_CASE( test_write_binary_data )
{
    TemporaryData data( "spikes" );
    brion::SpikeReport reportWrite( brion::URI( data.tmpFileName ),
                                    brion::MODE_WRITE );
    reportWrite.writeSpikes( data.spikes );
    reportWrite.close();

    const brion::SpikeReport reportRead( brion::URI( data.tmpFileName ),
                                         brion::MODE_READ );

    const brion::Spikes& readSpikes = reportRead.getSpikes();

    BOOST_CHECK_EQUAL_COLLECTIONS( data.spikes.begin(), data.spikes.end(),
                                   readSpikes.begin(), readSpikes.end( ));
}

BOOST_AUTO_TEST_CASE( test_write_existing_report )
{
    TemporaryData data( "dat" );

    brion::SpikeReport reportWrite( brion::URI( data.tmpFileName ),
                                    brion::MODE_WRITE );

    reportWrite.writeSpikes( data.spikes );
    reportWrite.close();

    BOOST_CHECK_THROW( brion::SpikeReport( brion::URI( data.tmpFileName ),
                                           brion::MODE_WRITE ),
                       std::runtime_error );
}

BOOST_AUTO_TEST_CASE( test_overwrite_flag_with_existing_report )
{
    TemporaryData data( "dat" );

    brion::SpikeReport reportWrite( brion::URI( data.tmpFileName ),
                                    brion::MODE_WRITE );

    reportWrite.writeSpikes( data.spikes );
    reportWrite.close();

    BOOST_CHECK_NO_THROW( brion::SpikeReport( brion::URI( data.tmpFileName ),
                                              brion::MODE_OVERWRITE ));
}

BOOST_AUTO_TEST_CASE( test_spikes_uri )
{
    TemporaryData data( "dat" );

    const brion::URI uri( data.tmpFileName );
    brion::SpikeReport report( uri, brion::MODE_WRITE );

    BOOST_CHECK_EQUAL( uri, report.getURI( ));
}
