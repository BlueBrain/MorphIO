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
#define BINARY_SPIKE_REPORT_FILE  "local/simulations/may17_2011/Control/out.spikes"


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



inline void debugSpikes(const brion::Spikes & v)
{
    for( auto& spike : v)
    {
        std::cout << spike.first << " -- " << spike.second <<std::endl;
    }
}



class TemporaryData
{
public:
    brion::Spikes spikes;
    const std::string tmpFileName;

    explicit TemporaryData( const std::string& reportType )
        : tmpFileName( "/tmp/" + lunchbox::make_UUID().getString() + "." +
                       reportType )
    {
        spikes.push_back({ 0.1f, 20 });
        spikes.push_back({ 0.2f, 22 });
        spikes.push_back({ 0.2f, 23 });
        spikes.push_back({ 0.3f, 24 });
        spikes.push_back({ 0.4f, 25 });
    }

    ~TemporaryData()
    {
        if( boost::filesystem::exists( tmpFileName ))
            boost::filesystem::remove( tmpFileName );
    }
};


// uri

BOOST_AUTO_TEST_CASE( spikes_uri )
{
    TemporaryData data( "dat" );

    const brion::URI uri { data.tmpFileName };
    brion::SpikeReport report { uri, brion::MODE_WRITE  };

    BOOST_CHECK_EQUAL( uri, report.getURI( ));
}

// invalid_extension
BOOST_AUTO_TEST_CASE( invalid_open_unknown_extension )
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


// invalid_open::file_notfound

BOOST_AUTO_TEST_CASE( invalid_open_file_notfound_binary )
{
    BOOST_CHECK_THROW( brion::SpikeReport report(
                           brion::URI( "/path/file.spikes" ), brion::MODE_READ ),
                       std::runtime_error );
}

BOOST_AUTO_TEST_CASE( invalid_open_file_notfound_bluron )
{
    BOOST_CHECK_THROW( brion::SpikeReport report( brion::URI( "/path/file.dat" ),
                                                  brion::MODE_READ ),
                       std::runtime_error );
}

BOOST_AUTO_TEST_CASE( invalid_open_file_notfound_nest )
{
    BOOST_CHECK_THROW( brion::SpikeReport report( brion::URI( "/path/file.gdf" ),
                                                  brion::MODE_READ ),
                       std::runtime_error );
}


BOOST_AUTO_TEST_CASE( bluron_invalid_report_information )
{
    boost::filesystem::path path( BBP_TESTDATA );
    path /= BLURON_SPIKE_REPORT_FILE;

    BOOST_CHECK_THROW( const brion::SpikeReport report(
                           brion::URI( path.string() + ";" + path.string( )),
                           brion::MODE_READ ),
                       std::runtime_error );
}

// invoke_invalid_method

BOOST_AUTO_TEST_CASE( invoke_invalid_method_binary )
{
    boost::filesystem::path path( BBP_TESTDATA );
    path /= BINARY_SPIKE_REPORT_FILE;

    brion::SpikeReport report( brion::URI( path.string( )),
                               brion::MODE_READ );
    BOOST_CHECK_THROW( report.write( brion::Spikes{ }),
                       std::runtime_error );
}


BOOST_AUTO_TEST_CASE( invoke_invalid_method_bluron )
{
    boost::filesystem::path path( BBP_TESTDATA );
    path /= BLURON_SPIKE_REPORT_FILE;

    brion::SpikeReport report( brion::URI( path.string( )),
                               brion::MODE_READ );
    BOOST_CHECK_THROW( report.write( brion::Spikes { }),
                       std::runtime_error );
}

BOOST_AUTO_TEST_CASE( invoke_invalid_method_nest )
{
    boost::filesystem::path path( BBP_TESTDATA );
    const std::string& files = "NESTSpikeData/spike_detector-65537-*.gdf";

    brion::SpikeReport report(
                brion::URI(( path / files ).string( )),
                brion::MODE_READ );
    BOOST_CHECK_THROW( report.write( brion::Spikes {} ),
                       std::runtime_error );
}

inline void testWrite (const char * format)
{
    TemporaryData data {format};
    {
        brion::SpikeReport report { brion::URI( data.tmpFileName ),
                                    brion::MODE_WRITE };
        report.write( data.spikes );
        report.close();
    }

    brion::SpikeReport report{ brion::URI( data.tmpFileName ),
                               brion::MODE_READ };
    brion::Spikes spikes = report.read(brion::UNDEFINED_TIMESTAMP).get();
    BOOST_CHECK_EQUAL_COLLECTIONS( data.spikes.begin(), data.spikes.end(),
                                   spikes.begin(), spikes.end( ));

    // test writing multiple chunks
    report = brion::SpikeReport{ brion::URI( data.tmpFileName ),
                                 brion::MODE_WRITE };
    report.write( brion::Spikes{ data.spikes.begin(),
                                 data.spikes.begin() + 3 });
    report.write( brion::Spikes{ data.spikes.begin() + 3,
                                 data.spikes.end( )});

    report = brion::SpikeReport{ brion::URI( data.tmpFileName ),
                                 brion::MODE_READ };
    spikes = report.read(brion::UNDEFINED_TIMESTAMP).get();
    BOOST_CHECK_EQUAL_COLLECTIONS( data.spikes.begin(), data.spikes.end(),
                                   spikes.begin(), spikes.end( ));
}

BOOST_AUTO_TEST_CASE( write_data_binary )
{
    testWrite( "spikes" );
}

BOOST_AUTO_TEST_CASE( write_data_nest )
{
    testWrite( "gdf" );
}

BOOST_AUTO_TEST_CASE( write_data_bluron )
{
    testWrite( "dat" );
}

inline void testRead(const char * format)
{

    TemporaryData data{ format } ;

    brion::SpikeReport reportWrite( brion::URI( data.tmpFileName ),
                                    brion::MODE_WRITE );
    reportWrite.write( data.spikes );
    reportWrite.close();

    brion::SpikeReport reportRead( brion::URI( data.tmpFileName ),
                                   brion::MODE_READ );

    // All the reports to be tested are file based, so they read until the end.
    auto spikes = reportRead.read( 0.3 ).get();
    BOOST_CHECK_EQUAL( spikes.size( ), 5 );
    BOOST_CHECK_EQUAL( reportRead.getCurrentTime(), brion::UNDEFINED_TIMESTAMP );
    BOOST_CHECK_EQUAL( reportRead.getState(), brion::SpikeReport::State::ended );

    BOOST_CHECK_THROW( reportRead.read( reportRead.getCurrentTime( )),
                       std::logic_error );
}

inline void testReadFiltered(const char * format)
{
    TemporaryData data{ format };

    brion::SpikeReport reportWrite( brion::URI( data.tmpFileName ),
                                    brion::MODE_WRITE );
    reportWrite.write( data.spikes );
    reportWrite.close();

    brion::SpikeReport reportRead( brion::URI( data.tmpFileName ),
                                   brion::GIDSet{ 22, 25 });

    // All the reports to be tested are file based, so they read until the end.
    auto spikes = reportRead.read( 0.3 ).get();
    BOOST_CHECK_EQUAL( spikes.size( ), 2 );
    BOOST_CHECK_EQUAL( reportRead.getCurrentTime(), brion::UNDEFINED_TIMESTAMP );
    BOOST_CHECK_EQUAL( reportRead.getState(), brion::SpikeReport::State::ended );
}

BOOST_AUTO_TEST_CASE( read_binary )
{
    testRead( "spikes" );
}

BOOST_AUTO_TEST_CASE( read_nest )
{
    testRead( "gdf" );
}

BOOST_AUTO_TEST_CASE( read_bluron )
{
    testRead( "dat" );
}

BOOST_AUTO_TEST_CASE( read_filtered_binary )
{
    testReadFiltered( "spikes" );
}

BOOST_AUTO_TEST_CASE( read_filtered_nest )
{
    testReadFiltered( "gdf" );
}

BOOST_AUTO_TEST_CASE( read_filtered_bluron )
{
    testReadFiltered( "dat" );
}

BOOST_AUTO_TEST_CASE( read_content_bluron )
{
    boost::filesystem::path path( BBP_TESTDATA );
    path /= BLURON_SPIKE_REPORT_FILE;

    brion::SpikeReport report( brion::URI( path.string( )), brion::MODE_READ );
    brion::Spikes spikes = report.read( brion::UNDEFINED_TIMESTAMP ).get();

    BOOST_REQUIRE_EQUAL( spikes.size(), BLURON_SPIKES_COUNT );

    BOOST_CHECK_EQUAL( spikes.begin()->first, BLURON_SPIKES_START_TIME );
    BOOST_CHECK_EQUAL( spikes.begin()->second, BLURON_FIRST_SPIKE_GID );

    BOOST_CHECK_EQUAL( spikes.rbegin()->first, BLURON_LAST_SPIKE_TIME );
    BOOST_CHECK_EQUAL( spikes.rbegin()->second, BLURON_LAST_SPIKE_GID );
}

BOOST_AUTO_TEST_CASE( read_content_nest )
{
    boost::filesystem::path path( BBP_TESTDATA );
    const std::string& files = "NESTSpikeData/spike_detector-65537-*.gdf";

    brion::SpikeReport report( brion::URI(( path / files ).string( )),
                               brion::MODE_READ );

    brion::Spikes spikes = report.read( brion::UNDEFINED_TIMESTAMP ).get();

    BOOST_REQUIRE_EQUAL( spikes.size(), NEST_SPIKES_COUNT );

    BOOST_CHECK_EQUAL( spikes.front().first, NEST_SPIKES_START_TIME );
    BOOST_CHECK_EQUAL( spikes.back().first, NEST_SPIKES_END_TIME );

    // The spikes are ordered by time but not by GIDs.
    // Extract the sorted set of GIDs corresponding to the first spike time.
    std::set<int> gids;
    std::vector<brion::Spike>::iterator it = spikes.begin();
    while( it != spikes.end() && it->first == NEST_SPIKES_START_TIME )
    {
        gids.insert( it->second );
        ++it;
    }
    BOOST_REQUIRE_EQUAL( gids.size(), NEST_FIRST_SPIKE_GID_COUNT );
    BOOST_CHECK_EQUAL( *gids.begin(), NEST_FIRST_SPIKE_GID );

    BOOST_CHECK_EQUAL( spikes.rbegin()->first, NEST_LAST_SPIKE_TIME );
    BOOST_CHECK_EQUAL( spikes.rbegin()->second, NEST_LAST_SPIKE_GID );
}

inline void testReadUntil(const char * format)
{
    TemporaryData data {format} ;

    brion::SpikeReport reportWrite( brion::URI( data.tmpFileName ),
                                    brion::MODE_WRITE );
    reportWrite.write( data.spikes );
    reportWrite.close();

    brion::SpikeReport reportRead( brion::URI( data.tmpFileName ),
                                   brion::MODE_READ );

    auto spikes = reportRead.readUntil( 0.25 ).get();
    BOOST_CHECK_EQUAL( spikes.size(), 3);
    BOOST_CHECK( reportRead.getCurrentTime() >= 0.25f );
    BOOST_CHECK( spikes.rbegin()->first < 0.25 );
    BOOST_CHECK_EQUAL( reportRead.getState(), brion::SpikeReport::State::ok );

    spikes = reportRead.read( brion::UNDEFINED_TIMESTAMP ).get();
    BOOST_CHECK_EQUAL( spikes.size(), 2);
    BOOST_CHECK_EQUAL( reportRead.getCurrentTime(), brion::UNDEFINED_TIMESTAMP );
    BOOST_CHECK_EQUAL( reportRead.getState(), brion::SpikeReport::State::ended );
}

inline void testReadUntilFiltered(const char * format)
{
    TemporaryData data {format} ;

    brion::SpikeReport reportWrite( brion::URI( data.tmpFileName ),
                                    brion::MODE_WRITE );
    reportWrite.write( data.spikes );
    reportWrite.close();

    brion::SpikeReport reportRead( brion::URI( data.tmpFileName ),
                                   brion::GIDSet{ 22, 25 });

    auto spikes = reportRead.readUntil( 0.25 ).get();
    BOOST_CHECK_EQUAL( spikes.size(), 1 );
    BOOST_CHECK( reportRead.getCurrentTime() >= 0.25f );
    BOOST_CHECK( spikes.rbegin()->first < 0.25 );
    BOOST_CHECK_EQUAL( reportRead.getState(), brion::SpikeReport::State::ok );

    spikes = reportRead.readUntil( brion::UNDEFINED_TIMESTAMP ).get();
    BOOST_CHECK_EQUAL( spikes.size(), 1 );
    BOOST_CHECK_EQUAL( reportRead.getCurrentTime(), brion::UNDEFINED_TIMESTAMP );
    BOOST_CHECK_EQUAL( reportRead.getState(), brion::SpikeReport::State::ended );

    BOOST_CHECK_THROW( reportRead.read( reportRead.getCurrentTime( )),
                       std::logic_error );
}

BOOST_AUTO_TEST_CASE( read_until_binary )
{
    testReadUntil( "spikes" );
}

BOOST_AUTO_TEST_CASE( read_until_nest )
{
    testReadUntil( "gdf" );
}

BOOST_AUTO_TEST_CASE( read_until_bluron )
{
    testReadUntil( "dat" );
}

BOOST_AUTO_TEST_CASE( read_until_filtered_binary )
{
    testReadUntilFiltered( "spikes" );
}

BOOST_AUTO_TEST_CASE( read_until_filtered_nest )
{
    testReadUntilFiltered( "gdf" );
}

BOOST_AUTO_TEST_CASE( read_until_filtered_bluron )
{
    testReadUntilFiltered( "dat" );
}

// read_seek

inline void testReadSeek(const char* format)
{
    TemporaryData data{ format };

    brion::SpikeReport reportWrite{
        brion::URI( data.tmpFileName ), brion::MODE_WRITE };
    reportWrite.write( data.spikes );
    reportWrite.close();

    brion::SpikeReport reportRead{ brion::URI( data.tmpFileName ),
                                   brion::MODE_READ };
    reportRead.seek( 0.3f ).get();

    BOOST_CHECK_EQUAL( reportRead.getCurrentTime(), 0.3f );

    auto spikes = reportRead.read( brion::UNDEFINED_TIMESTAMP ).get();

    BOOST_CHECK_EQUAL( spikes.size(), 2 );
    BOOST_CHECK_EQUAL( reportRead.getCurrentTime(), brion::UNDEFINED_TIMESTAMP );
    BOOST_CHECK_EQUAL( reportRead.getState(), brion::SpikeReport::State::ended );

    reportRead.seek( 0.25f ).get();
    BOOST_CHECK_EQUAL( reportRead.getCurrentTime(), 0.25f );
    BOOST_CHECK_EQUAL( reportRead.getState(), brion::SpikeReport::State::ok );

    spikes = reportRead.read( brion::UNDEFINED_TIMESTAMP ).get();
    BOOST_CHECK_EQUAL( spikes.size(), 2 );
    BOOST_CHECK_EQUAL( reportRead.getState(), brion::SpikeReport::State::ended );

    reportRead.seek( -2.f ).get();
    BOOST_CHECK_EQUAL( reportRead.getCurrentTime(), -2.f );
    BOOST_CHECK_EQUAL( reportRead.getState(), brion::SpikeReport::State::ok );

    spikes = reportRead.read( brion::UNDEFINED_TIMESTAMP ).get();
    BOOST_CHECK_EQUAL( spikes.size(), 5 );
    BOOST_CHECK_EQUAL( reportRead.getState(), brion::SpikeReport::State::ended );

    reportRead.seek( 10.f ).get();
    BOOST_CHECK_EQUAL( reportRead.getCurrentTime(), brion::UNDEFINED_TIMESTAMP );
    BOOST_CHECK_EQUAL( reportRead.getState(), brion::SpikeReport::State::ended );
}

BOOST_AUTO_TEST_CASE( read_seek_binary )
{
    testReadSeek( "spikes" );
}

BOOST_AUTO_TEST_CASE( read_seek_nest )
{
    testReadSeek( "gdf" );
}

BOOST_AUTO_TEST_CASE( read_seek_bluron )
{
    testReadSeek( "dat" );
}

// invalid_read

inline void testInvalidRead(const char * format)
{
    TemporaryData data{ format };

    brion::SpikeReport reportWrite{ brion::URI( data.tmpFileName ),
                                    brion::MODE_WRITE };
    reportWrite.write( data.spikes );
    reportWrite.close();

    brion::SpikeReport reportRead{ brion::URI( data.tmpFileName ),
                                   brion::MODE_READ };
    reportRead.readUntil( 0.3 ).get();

    BOOST_CHECK_THROW( reportRead.read( 0.1 ), std::logic_error );
    BOOST_CHECK_THROW( reportRead.readUntil( 0.1 ), std::logic_error );
}

BOOST_AUTO_TEST_CASE( invalid_read_binary )
{
    testInvalidRead( "spikes" );
}

BOOST_AUTO_TEST_CASE( invalid_read_nest )
{
    testInvalidRead( "gdf" );
}

BOOST_AUTO_TEST_CASE( invalid_read_bluron )
{
    testInvalidRead( "dat" );
}

// invalid write

inline void testInvalidWrite(const char * format)
{
    TemporaryData data{ format };
    brion::SpikeReport reportWrite( brion::URI( data.tmpFileName ),
                                    brion::MODE_WRITE );
    reportWrite.write( data.spikes );

    BOOST_CHECK_THROW( reportWrite.write({{ 0.0, 0 }}), std::logic_error );
    
    BOOST_CHECK_THROW( reportWrite.write(
                           {{10.0, 0}, {10.0, 1}, {11.0, 0}, {0.5, 1}}),
                       std::logic_error);

    brion::SpikeReport reportRead { brion::URI( data.tmpFileName ),
                                    brion::MODE_READ };
    BOOST_CHECK_THROW( reportRead.write({{ 100.0, 0 }}), std::runtime_error );
}

BOOST_AUTO_TEST_CASE( invalid_write_binary )
{
    testInvalidWrite( "spikes" );
}

BOOST_AUTO_TEST_CASE( invalid_write_nest )
{
    testInvalidWrite( "gdf" );
}

BOOST_AUTO_TEST_CASE( invalid_write_bluron )
{
    testInvalidWrite( "dat" );
}

// write incremental

inline void testWriteIncremental (const char * format)
{
    TemporaryData data{ format };
    brion::SpikeReport reportWrite( brion::URI( data.tmpFileName ),
                                    brion::MODE_WRITE );
    reportWrite.write({{ 0.1, 1 }});
    reportWrite.write({{ 0.2, 1 }});
    reportWrite.write({{ 0.3, 1 }});
    reportWrite.close();

    brion::SpikeReport reportRead( brion::URI( data.tmpFileName ),
                                   brion::MODE_READ );

    brion::Spikes spikes = reportRead.read( brion::UNDEFINED_TIMESTAMP ).get();
    BOOST_CHECK_EQUAL( spikes.size(), 3 );
}

BOOST_AUTO_TEST_CASE( write_incremental_binary )
{
    testWriteIncremental( "spikes" );
}

BOOST_AUTO_TEST_CASE( write_incremental_nest )
{
    testWriteIncremental( "gdf" );
}

BOOST_AUTO_TEST_CASE( write_incremental_bluron )
{
    testWriteIncremental( "dat" );
}

// seek and write

inline void testSeekAndWrite( const char * format )
{
    TemporaryData data{ format };
    brion::SpikeReport reportWrite( brion::URI( data.tmpFileName ),
                                    brion::MODE_WRITE );

    reportWrite.write({{ 0.1 ,1 }});
    reportWrite.write({{ 0.2 ,1 }});
    reportWrite.write({{ 0.3 ,1 }});

    reportWrite.seek( 0.2 ).get();

    reportWrite.write({{ 0.4 , 1 }});
    reportWrite.write({{ 0.8 , 1 }});

    reportWrite.close();

    brion::SpikeReport reportRead( brion::URI( data.tmpFileName ),
                                   brion::MODE_READ );

    brion::Spikes spikes = reportRead.read( brion::UNDEFINED_TIMESTAMP ).get();

    static const brion::Spikes expected = {{ 0.1 , 1 }, { 0.4 ,1 }, { 0.8 ,1 }};

    BOOST_CHECK_EQUAL_COLLECTIONS( spikes.begin(), spikes.end(),
                                   expected.begin(), expected.end( ));
}

BOOST_AUTO_TEST_CASE( seek_and_write_binary )
{
    testSeekAndWrite( "spikes" );
}

BOOST_AUTO_TEST_CASE( seek_and_write_nest )
{
    // Not supported
}

BOOST_AUTO_TEST_CASE( seek_and_write_bluron )
{
    // Not supported
}
