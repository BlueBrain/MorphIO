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
 * This file is part of Brion <https://github.com/BlueBrain/Brion>
 */

#include <brion/brion.h>
#include <BBP/TestDatasets.h>

#define BOOST_TEST_MODULE CompartmentReport
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/foreach.hpp>
#include <boost/test/unit_test.hpp>
#include <lunchbox/log.h>

using boost::lexical_cast;

BOOST_AUTO_TEST_CASE( test_invalid_open )
{
    BOOST_CHECK_THROW( brion::CompartmentReport( brion::URI( "/bla" ),
                                                 brion::MODE_READ ),
                       std::runtime_error );
    BOOST_CHECK_THROW( brion::CompartmentReport( brion::URI( "bla" ),
                                                 brion::MODE_READ ),
                       std::runtime_error );

    boost::filesystem::path path( BBP_TESTDATA );
    path /= "local/README";
    BOOST_CHECK_THROW( brion::CompartmentReport( brion::URI( path.string( )),
                                                 brion::MODE_READ ),
                       std::runtime_error );

    path = BBP_TESTDATA;
    path /= "local/morphologies/01.07.08/h5/R-C010306G.h5";
    BOOST_CHECK_THROW( brion::CompartmentReport( brion::URI( path.string( )),
                                                 brion::MODE_READ ),
                       std::runtime_error );
}

BOOST_AUTO_TEST_CASE( test_open_binary )
{
    boost::filesystem::path path( BBP_TESTDATA );
    path /= "local/simulations/may17_2011/Control/voltage.bbp";
    BOOST_CHECK_NO_THROW( brion::CompartmentReport( brion::URI( path.string( )),
                                                    brion::MODE_READ ));
}

BOOST_AUTO_TEST_CASE( test_open_hdf5 )
{
    boost::filesystem::path path( BBP_TESTDATA );
    path /= "local/simulations/may17_2011/Control/voltage.h5";
    BOOST_CHECK_NO_THROW( brion::CompartmentReport( brion::URI( path.string( )),
                                                    brion::MODE_READ ));
}

BOOST_AUTO_TEST_CASE( test_invalid_mapping )
{
    boost::filesystem::path path( BBP_TESTDATA );
    path /= "local/simulations/may17_2011/Control/voltage.bbp";
    brion::GIDSet gids;
    gids.insert( 123456789 );
    BOOST_CHECK_THROW( brion::CompartmentReport( brion::URI( path.string( )),
                                                 brion::MODE_READ, gids ),
                       std::runtime_error );
}

namespace
{
boost::filesystem::path createUniquePath()
{
    return boost::filesystem::unique_path();
}
}

BOOST_AUTO_TEST_CASE( test_create_write_report )
{
    struct Fixture
    {
        Fixture()
            : temp(createUniquePath())
            , h5(temp.string() + ".h5")
            , bin(temp.string() + ".bin")
        {}

        ~Fixture()
        {
            boost::filesystem::remove( lexical_cast< std::string >( h5 ));
            boost::filesystem::remove( lexical_cast< std::string >( bin ));
        }
        boost::filesystem::path temp;
        const brion::URI h5;
        const brion::URI bin;
    } fixture;


    {
        // A separate block is needed to close the report. Otherwise it cannot
        // be reopened from rewriting.
        BOOST_CHECK_NO_THROW(
                    brion::CompartmentReport report( fixture.h5,
                                                     brion::MODE_OVERWRITE ));
    }
    BOOST_CHECK_THROW(
                brion::CompartmentReport( fixture.h5, brion::MODE_WRITE ),
                std::runtime_error );
    BOOST_CHECK_NO_THROW( brion::CompartmentReport report2( fixture.h5,
                                                       brion::MODE_OVERWRITE ));
    BOOST_CHECK_THROW(
                brion::CompartmentReport( fixture.bin, brion::MODE_WRITE ),
                std::runtime_error );
}

BOOST_AUTO_TEST_CASE( test_bounds_binary )
{
    boost::filesystem::path path( BBP_TESTDATA );
    path /= "local/simulations/may17_2011/Control/voltage.bbp";

    brion::GIDSet gids;
    gids.insert( 1 );
    brion::CompartmentReport report( brion::URI( path.string( )),
                                     brion::MODE_READ, gids );
    brion::floatsPtr frame;

    frame = report.loadFrame( report.getStartTime( ));
    BOOST_CHECK( frame );

    frame = report.loadFrame( report.getEndTime( ));
    BOOST_CHECK( frame );

    frame = report.loadFrame( report.getEndTime()+1);
    BOOST_CHECK( !frame );
}

BOOST_AUTO_TEST_CASE( test_bounds_hdf5 )
{
    boost::filesystem::path path( BBP_TESTDATA );
    path /= "local/simulations/may17_2011/Control/voltage.h5";

    brion::GIDSet gids;
    gids.insert( 1 );
    brion::CompartmentReport report( brion::URI( path.string( )),
                                     brion::MODE_READ, gids );
    brion::floatsPtr frame;

    frame = report.loadFrame( report.getStartTime( ));
    BOOST_CHECK( frame );

    frame = report.loadFrame( report.getEndTime( ));
    BOOST_CHECK( frame );

    frame = report.loadFrame( report.getEndTime()+1);
    BOOST_CHECK( !frame );
}

void test_compare( const brion::URI& uri1, const brion::URI& uri2 )
{
    std::cout << "Compare " << uri1 << " == " << uri2 << std::endl;

    brion::CompartmentReport report1( uri1, brion::MODE_READ );
    brion::CompartmentReport report2( uri2, brion::MODE_READ );

    BOOST_CHECK_EQUAL( report1.getStartTime(), report2.getStartTime( ));
    BOOST_CHECK_EQUAL( report1.getEndTime(), report2.getEndTime( ));
    BOOST_CHECK_EQUAL( report1.getTimestep(), report2.getTimestep( ));
    BOOST_CHECK_EQUAL( report1.getFrameSize(), report2.getFrameSize( ));
    BOOST_CHECK_EQUAL_COLLECTIONS(
        report1.getGIDs().begin(), report1.getGIDs().end(),
        report2.getGIDs().begin(), report2.getGIDs().end( ));
    BOOST_CHECK_EQUAL( report1.getDataUnit(), report2.getDataUnit( ));
    BOOST_CHECK_EQUAL( report1.getTimeUnit(), report2.getTimeUnit( ));
    BOOST_CHECK( !report1.getDataUnit().empty( ));
    BOOST_CHECK( !report1.getTimeUnit().empty( ));

    const brion::SectionOffsets& offsets1 = report1.getOffsets();
    const brion::SectionOffsets& offsets2 = report2.getOffsets();
    const brion::CompartmentCounts& counts1 = report1.getCompartmentCounts();
    const brion::CompartmentCounts& counts2 = report2.getCompartmentCounts();

    BOOST_CHECK_EQUAL( offsets1.size(), offsets2.size( ));
    BOOST_CHECK_EQUAL( counts1.size(), counts2.size( ));

    for( size_t i = 0; i < offsets1.size(); ++i )
    {
        BOOST_CHECK_EQUAL_COLLECTIONS(
            offsets1[i].begin(), offsets1[i].end(),
            offsets2[i].begin(), offsets2[i].end( ));

        for( size_t j = 0; j < offsets1[i].size(); ++j )
            BOOST_CHECK( offsets1[i][j] < report1.getFrameSize() ||
                     offsets1[i][j] == std::numeric_limits< uint64_t >::max( ));
    }

    for( float i = report1.getStartTime(); i < report1.getEndTime();
         i += report1.getTimestep( ))
    {
        brion::floatsPtr frame1 = report1.loadFrame( i );
        brion::floatsPtr frame2 = report2.loadFrame( i );

        BOOST_CHECK( frame1 );
        BOOST_CHECK( frame2 );

        for( size_t j = 0; j < report1.getFrameSize(); ++j )
        {
            BOOST_CHECK_EQUAL( (*frame1)[j], (*frame2)[j] );
        }
    }

    try
    {
        // cross-check second GID's voltage report
        const uint32_t gid = *( ++report1.getGIDs().begin( ));
        brion::floatsPtr frame1 = report1.loadNeuron( gid );
        brion::floatsPtr frame2 = report2.loadNeuron( gid );
        const size_t size = report1.getNeuronSize( gid );

        BOOST_CHECK( frame1 );
        BOOST_CHECK( frame2 );
        BOOST_CHECK( size > 0  );
        BOOST_CHECK_EQUAL( size, report2.getNeuronSize( gid ));

        for( size_t i = 0; i < size; ++i )
        {
            BOOST_CHECK_EQUAL( (*frame1)[i], (*frame2)[i] );
        }
    }
    catch( const std::runtime_error& ) {} // loadNeuron(gid) is optional, ignore

    brion::GIDSet gids;
    gids.insert( 394 );
    report1.updateMapping( gids );
    report2.updateMapping( gids );

    BOOST_CHECK_EQUAL( report1.getGIDs().size(), 1 );
    BOOST_CHECK_EQUAL( *report1.getGIDs().begin(), 394 );
    BOOST_CHECK_EQUAL( report2.getGIDs().size(), 1 );
    BOOST_CHECK_EQUAL( *report2.getGIDs().begin(), 394 );
}

/** @return false if no "from" or "to" report plugin was found, true otherwise*/
bool convert( const brion::URI& fromURI, const brion::URI& toURI )
{
    try
    {
        std::cout << "Convert " << fromURI << " -> " << toURI << std::endl;
        namespace bp = boost::posix_time;
        const bp::ptime startTime = bp::microsec_clock::local_time();

        const brion::CompartmentReport from( fromURI, brion::MODE_READ );

        const float start = from.getStartTime();
        const float end = from.getEndTime();
        const float step = from.getTimestep();
        BOOST_CHECK( start >= 0.f );
        BOOST_CHECK( start < end );
        BOOST_CHECK_NE( step, 0.f );
        BOOST_CHECK( !from.getDataUnit().empty( ));
        BOOST_CHECK( !from.getTimeUnit().empty( ));

        brion::CompartmentReport to( toURI, brion::MODE_OVERWRITE );
        to.writeHeader( start, end, step,
                        from.getDataUnit(), from.getTimeUnit( ));

        const brion::CompartmentCounts& counts = from.getCompartmentCounts();
        const brion::GIDSet& gids = from.getGIDs();
        BOOST_CHECK_EQUAL( gids.size(), counts.size( ));

        size_t i = 0;
        BOOST_FOREACH( const uint32_t gid, gids )
        {
            BOOST_CHECK( !counts[ i ].empty( ));
            BOOST_CHECK( to.writeCompartments( gid, counts[ i++ ] ));
        }

        for( float t = start; t < end; t += step )
        {
            brion::floatsPtr data = from.loadFrame( t );
            const brion::floats& voltages = *data.get();
            const brion::SectionOffsets& offsets = from.getOffsets();

            BOOST_CHECK( data != 0 );
            BOOST_CHECK_EQUAL( offsets.size(), gids.size( ));

            i = 0;
            BOOST_FOREACH( const uint32_t gid, gids )
            {
                brion::floats cellVoltages;
                cellVoltages.reserve( from.getNumCompartments( i ));

                for( size_t j = 0; j < offsets[i].size(); ++j )
                    for( size_t k = 0; k < counts[i][j]; ++k )
                        cellVoltages.push_back( voltages[ offsets[i][j]+k] );

                BOOST_CHECK_EQUAL( cellVoltages.size(),
                                   from.getNumCompartments( i ));
                BOOST_CHECK( !cellVoltages.empty( ));
                BOOST_CHECK( to.writeFrame( gid, cellVoltages, t ));
                ++i;
            }
        }
        std::cout << ( bp::microsec_clock::local_time() -
                       startTime ).total_milliseconds() << " ms" << std::endl;
        return true;
    }
    catch( const std::runtime_error& e )
    {
        const std::string expected( "No plugin implementation available for " );
        BOOST_CHECK_MESSAGE( std::string( e.what( )).find( expected ) == 0,
                             e.what( ));
        return false;
    }
}

void testPerf( const brion::URI& uri )
{
    namespace bp = boost::posix_time;
    bp::ptime startTime = bp::microsec_clock::local_time();
    const brion::CompartmentReport report( uri, brion::MODE_READ );
    const bp::time_duration openTime = bp::microsec_clock::local_time() -
                                       startTime;

    startTime = bp::microsec_clock::local_time();
    for( float i = report.getStartTime(); i < report.getEndTime();
         i += report.getTimestep( ))
    {
        brion::floatsPtr frame = report.loadFrame( i );
        BOOST_CHECK( frame );
    }
    const bp::time_duration readTime = bp::microsec_clock::local_time() -
                                       startTime;

    std::cout << uri << ": open " << openTime.total_milliseconds() << ", load "
              << readTime.total_milliseconds() << " ms" << std::endl;
}

BOOST_AUTO_TEST_CASE( test_convert_and_compare )
{
    boost::filesystem::path path( BBP_TESTDATA );
    path /= "local/simulations/may17_2011/Control/";
    const brion::URI source( path.string() + "allCompartments.bbp" );

    test_compare( source, brion::URI( path.string() + "allCompartments.h5" ));

    const boost::filesystem::path& temp = createUniquePath();
    std::vector< brion::URI > uris;

    uris.push_back( brion::URI( temp.string() + ".h5" ));
    uris.push_back( brion::URI( "leveldb://" ));
    uris.push_back( brion::URI( "memcached:///briontest" ));

    while( !uris.empty( ))
    {
        const brion::URI first = uris.back();
        uris.pop_back();

        if( convert( source, first )) // bootstrap first from source
        {
            test_compare( source, first );
            testPerf( first );

            BOOST_FOREACH( const brion::URI& second, uris )
            {
                if( convert( source, second )) // bootstrap second from source
                {
                    test_compare( first, second );
                    if( convert( second, first ))
                        test_compare( source, first );
                    if( convert( first, second ))
                        test_compare( source, second );
                }
            }
        }
    }
}

BOOST_AUTO_TEST_CASE( test_read_soma_binary )
{
    boost::filesystem::path path( BBP_TESTDATA );
    path /= "local/simulations/may17_2011/Control/voltage.bbp";

    brion::GIDSet gids;
    gids.insert( 1 );
    brion::CompartmentReport report( brion::URI( path.string( )),
                                     brion::MODE_READ, gids );

    BOOST_CHECK_EQUAL( report.getStartTime(), 0.f );
    BOOST_CHECK_EQUAL( report.getEndTime(), 10.f );
    BOOST_CHECK_EQUAL( report.getTimestep(), 0.1f );
    BOOST_CHECK_EQUAL( report.getFrameSize(), 1 );

    brion::floatsPtr frame = report.loadFrame( report.getStartTime( ));
    BOOST_CHECK( frame );
    BOOST_CHECK_EQUAL( (*frame)[0], -65 );

    frame = report.loadFrame( 4.5f );
    BOOST_CHECK( frame );
    BOOST_CHECK_CLOSE( (*frame)[0], -10.1440039f, .000001f );
}

BOOST_AUTO_TEST_CASE( test_read_soma_hdf5 )
{
    boost::filesystem::path path( BBP_TESTDATA );
    path /= "local/simulations/may17_2011/Control/voltage.h5";

    brion::GIDSet gids;
    gids.insert( 1 );
    brion::CompartmentReport report( brion::URI( path.string( )),
                                     brion::MODE_READ, gids );

    BOOST_CHECK_EQUAL( report.getStartTime(), 0.f );
    BOOST_CHECK_EQUAL( report.getEndTime(), 10.f );
    BOOST_CHECK_EQUAL( report.getTimestep(), 0.1f );
    BOOST_CHECK_EQUAL( report.getFrameSize(), 1 );

    brion::floatsPtr frame = report.loadFrame( report.getStartTime( ));
    BOOST_CHECK( frame );
    BOOST_CHECK_EQUAL( (*frame)[0], -65 );

    frame = report.loadFrame( 4.5f );
    BOOST_CHECK( frame );
    BOOST_CHECK_CLOSE( (*frame)[0], -10.1440039f, .000001f );
}

BOOST_AUTO_TEST_CASE( test_read_allcomps_binary )
{
    boost::filesystem::path path( BBP_TESTDATA );
    path /= "local/simulations/may17_2011/Control/allCompartments.bbp";
    brion::CompartmentReport report( brion::URI( path.string( )),
                                     brion::MODE_READ );

    BOOST_CHECK_EQUAL( report.getStartTime(), 0.f );
    BOOST_CHECK_EQUAL( report.getEndTime(), 10.f );
    BOOST_CHECK_EQUAL( report.getTimestep(), 0.1f );
    BOOST_CHECK_EQUAL( report.getFrameSize(), 20360 );

    brion::floatsPtr frame = report.loadFrame( .8f );
    BOOST_CHECK( frame );
    BOOST_CHECK_CLOSE( (*frame)[0], -65.2919388, .000001f );
    BOOST_CHECK_CLOSE( (*frame)[1578], -65.2070618, .000001f );

    brion::GIDSet gids;
    gids.insert( 394 );
    report.updateMapping( gids );

    BOOST_CHECK_EQUAL( report.getStartTime(), 0.f );
    BOOST_CHECK_EQUAL( report.getEndTime(), 10.f );
    BOOST_CHECK_EQUAL( report.getTimestep(), 0.1f );
    BOOST_CHECK_EQUAL( report.getFrameSize(), 629 );

    frame = report.loadFrame( report.getStartTime( ));
    BOOST_CHECK( frame );
    BOOST_CHECK_EQUAL( (*frame)[0], -65 );

    frame = report.loadFrame( 4.5f );
    BOOST_CHECK( frame );
    BOOST_CHECK_CLOSE( (*frame)[0], -65.3935928f, .000001f );
}

BOOST_AUTO_TEST_CASE( test_read_allcomps_hdf5 )
{
    boost::filesystem::path path( BBP_TESTDATA );
    path /= "local/simulations/may17_2011/Control/allCompartments.h5";
    brion::CompartmentReport report( brion::URI( path.string( )),
                                     brion::MODE_READ );

    BOOST_CHECK_EQUAL( report.getStartTime(), 0.f );
    BOOST_CHECK_EQUAL( report.getEndTime(), 10.f );
    BOOST_CHECK_EQUAL( report.getTimestep(), 0.1f );
    BOOST_CHECK_EQUAL( report.getFrameSize(), 20360 );

    brion::floatsPtr frame = report.loadFrame( .8f );
    BOOST_CHECK( frame );
    BOOST_CHECK_CLOSE( (*frame)[0], -65.2919388, .000001f );
    BOOST_CHECK_CLOSE( (*frame)[1578], -65.2070618, .000001f );

    brion::GIDSet gids;
    gids.insert( 394 );
    report.updateMapping( gids );

    BOOST_CHECK_EQUAL( report.getStartTime(), 0.f );
    BOOST_CHECK_EQUAL( report.getEndTime(), 10.f );
    BOOST_CHECK_EQUAL( report.getTimestep(), 0.1f );
    BOOST_CHECK_EQUAL( report.getFrameSize(), 629 );

    frame = report.loadFrame( report.getStartTime( ));
    BOOST_CHECK( frame );
    BOOST_CHECK_EQUAL( (*frame)[0], -65 );

    frame = report.loadFrame( 4.5f );
    BOOST_CHECK( frame );
    BOOST_CHECK_CLOSE( (*frame)[0], -65.3935928f, .000001f );
}

BOOST_AUTO_TEST_CASE( test_read_subtarget_binary )
{
    boost::filesystem::path path( BBP_TESTDATA );
    path /= "local/simulations/may17_2011/Control/allCompartments.bbp";

    brion::GIDSet gids;
    gids.insert( 394 );
    gids.insert( 400 );
    brion::CompartmentReport report( brion::URI( path.string( )),
                                     brion::MODE_READ, gids );

    const brion::SectionOffsets& offsets = report.getOffsets();
    BOOST_CHECK_EQUAL( offsets.size(), 2 );

    BOOST_CHECK_EQUAL( report.getStartTime(), 0.f );
    BOOST_CHECK_EQUAL( report.getEndTime(), 10.f );
    BOOST_CHECK_EQUAL( report.getTimestep(), 0.1f );
    BOOST_CHECK_EQUAL( report.getFrameSize(), 938 );

    brion::floatsPtr frame = report.loadFrame( report.getStartTime( ));
    BOOST_CHECK( frame );
    BOOST_CHECK_EQUAL( (*frame)[offsets[0][0]], -65 );
    BOOST_CHECK_EQUAL( (*frame)[offsets[1][0]], -65 );
    BOOST_CHECK_EQUAL( (*frame)[offsets[0][1]], -65 );
    BOOST_CHECK_EQUAL( (*frame)[offsets[1][1]], -65 );

    frame = report.loadFrame( 4.5f );
    BOOST_CHECK( frame );
    BOOST_CHECK_CLOSE( (*frame)[offsets[0][0]], -65.3935928f, .000001f );
    BOOST_CHECK_CLOSE( (*frame)[offsets[1][0]], -65.9297104f, .000001f );
    BOOST_CHECK_CLOSE( (*frame)[offsets[0][1]], -65.4166641f, .000001f );
    BOOST_CHECK_CLOSE( (*frame)[offsets[1][1]], -65.9334106f, .000001f );
}

BOOST_AUTO_TEST_CASE( test_read_subtarget_hdf5 )
{
    boost::filesystem::path path( BBP_TESTDATA );
    path /= "local/simulations/may17_2011/Control/allCompartments.h5";

    brion::GIDSet gids;
    gids.insert( 394 );
    gids.insert( 400 );
    brion::CompartmentReport report( brion::URI( path.string( )),
                                     brion::MODE_READ, gids );

    const brion::SectionOffsets& offsets = report.getOffsets();
    BOOST_CHECK_EQUAL( offsets.size(), 2 );

    BOOST_CHECK_EQUAL( report.getStartTime(), 0.f );
    BOOST_CHECK_EQUAL( report.getEndTime(), 10.f );
    BOOST_CHECK_EQUAL( report.getTimestep(), 0.1f );
    BOOST_CHECK_EQUAL( report.getFrameSize(), 938 );

    brion::floatsPtr frame = report.loadFrame( report.getStartTime( ));
    BOOST_CHECK( frame );
    BOOST_CHECK_EQUAL( (*frame)[offsets[0][0]], -65 );
    BOOST_CHECK_EQUAL( (*frame)[offsets[1][0]], -65 );
    BOOST_CHECK_EQUAL( (*frame)[offsets[0][1]], -65 );
    BOOST_CHECK_EQUAL( (*frame)[offsets[1][1]], -65 );

    frame = report.loadFrame( 4.5f );
    BOOST_CHECK( frame );
    BOOST_CHECK_CLOSE( (*frame)[offsets[0][0]], -65.3935928f, .000001f );
    BOOST_CHECK_CLOSE( (*frame)[offsets[1][0]], -65.9297104f, .000001f );
    BOOST_CHECK_CLOSE( (*frame)[offsets[0][1]], -65.4166641f, .000001f );
    BOOST_CHECK_CLOSE( (*frame)[offsets[1][1]], -65.9334106f, .000001f );
}

BOOST_AUTO_TEST_CASE( test_perf_binary )
{
    boost::filesystem::path path( BBP_TESTDATA );
    path /= "local/simulations/may17_2011/Control/allCompartments.bbp";
    testPerf( brion::URI( path.string( )));
}

BOOST_AUTO_TEST_CASE( test_perf_hdf5 )
{
    boost::filesystem::path path( BBP_TESTDATA );
    path /= "local/simulations/may17_2011/Control/allCompartments.h5";
    testPerf( brion::URI( path.string( )));
}
