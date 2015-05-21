/* Copyright (c) 2014-2015, EPFL/Blue Brain Project
 *                          Juan Hernando <jhernando@fi.upm.es>
 *
 * This file is part of Brion <https://github.com/BlueBrain/Brion>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 * - Neither the name of Eyescale Software GmbH nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "../brion/plugin/spikeReportSimpleStreamer.h"

#include <brion/brion.h>
#include <BBP/TestDatasets.h>
#include <lunchbox/pluginRegisterer.h>

#define BOOST_TEST_MODULE SpikeReport
#include <boost/filesystem/path.hpp>
#include <boost/test/unit_test.hpp>

#define NEST_SPIKE_REPORT_FILE "NESTSpikeData/spike_detector-65537-00.gdf"
// These constants refer to the single file that is being loaded
#define NEST_SPIKES_START_TIME 1.8f
#define NEST_SPIKES_END_TIME 98.8f
#define NEST_SPIKES_COUNT 96256

using brion::UNDEFINED_TIMESTAMP;

/* This plugin is for testing purposes. For that reason it doesn't register
   itself */
lunchbox::PluginRegisterer< brion::plugin::SpikeReportSimpleStreamer >
    registerer;

BOOST_AUTO_TEST_CASE( test_stream_open )
{
    boost::filesystem::path path( BBP_TESTDATA );
    path /= NEST_SPIKE_REPORT_FILE;
    brion::SpikeReport report( brion::URI( "spikes://" + path.string( )),
                               brion::MODE_READ);

    BOOST_CHECK_EQUAL( report.getReadMode(),
                       brion::SpikeReport::STREAM );
}

BOOST_AUTO_TEST_CASE( test_stream_close )
{
    boost::filesystem::path path( BBP_TESTDATA );
    path /= NEST_SPIKE_REPORT_FILE;
    brion::SpikeReport report( brion::URI( "spikes://" + path.string( )),
                               brion::MODE_READ);
    report.close();
    // It's not possible to properly test concurrent waitUntil and close
    // because SpikeReportSimpleStreamer finishes too fast and never makes
    // waitUntil wait forever.
    // At least we test the return value.
    BOOST_CHECK( !report.waitUntil( UNDEFINED_TIMESTAMP ));
}

BOOST_AUTO_TEST_CASE( test_stream_no_read )
{
    boost::filesystem::path path( BBP_TESTDATA );
    path /= NEST_SPIKE_REPORT_FILE;
    brion::SpikeReport report( brion::URI( "spikes://" + path.string( )),
                               brion::MODE_READ);

    BOOST_CHECK_EQUAL( report.getStartTime(), UNDEFINED_TIMESTAMP );
    BOOST_CHECK_EQUAL( report.getEndTime(), UNDEFINED_TIMESTAMP );
    BOOST_CHECK( report.getSpikes().empty( ));
}

BOOST_AUTO_TEST_CASE( test_stream_read_all )
{
    boost::filesystem::path path( BBP_TESTDATA );
    path /= NEST_SPIKE_REPORT_FILE;
    brion::SpikeReport report( brion::URI( "spikes://" + path.string( )),
                               brion::MODE_READ);

    report.waitUntil( UNDEFINED_TIMESTAMP );

    BOOST_CHECK_EQUAL( report.getStartTime(), NEST_SPIKES_START_TIME );
    BOOST_CHECK_EQUAL( report.getEndTime(), NEST_SPIKES_END_TIME );
    BOOST_CHECK_EQUAL( report.getSpikes().size(), NEST_SPIKES_COUNT );
}

BOOST_AUTO_TEST_CASE( test_stream_read_timeout )
{
    boost::filesystem::path path( BBP_TESTDATA );
    path /= NEST_SPIKE_REPORT_FILE;
    brion::SpikeReport report( brion::URI( "spikes://" + path.string( )),
                               brion::MODE_READ);

    BOOST_CHECK( !report.waitUntil( 1000, 1 ));
}

BOOST_AUTO_TEST_CASE( test_stream_read_by_chunks )
{
    boost::filesystem::path path( BBP_TESTDATA );
    path /= NEST_SPIKE_REPORT_FILE;
    brion::SpikeReport report( brion::URI( "spikes://" + path.string( )),
                               brion::MODE_READ );

    BOOST_CHECK( report.waitUntil( 0 ));
    BOOST_CHECK_EQUAL( report.getEndTime(), UNDEFINED_TIMESTAMP);
    BOOST_CHECK( report.getSpikes().empty( ));

    for( float time = 10; time < 101; time += 10 )
    {
        const bool status = report.waitUntil( time );
        if( status )
            BOOST_CHECK( report.getEndTime() <= time );
        else
        {
            // In this test status is only False when the end of the stream
            // was reached.
            BOOST_CHECK_EQUAL( report.getEndTime(), NEST_SPIKES_END_TIME );
        }
    }
}

BOOST_AUTO_TEST_CASE( test_stream_read_by_chunks_with_timeout )
{
    boost::filesystem::path path( BBP_TESTDATA );
    path /= NEST_SPIKE_REPORT_FILE;
    brion::SpikeReport report( brion::URI( "spikes://" + path.string( )),
                               brion::MODE_READ);

    for( float time = 10; time < 101; ++time )
    {
        while( !report.waitUntil( time, 1 ))
        {
            if( report.getEndTime() == NEST_SPIKES_END_TIME )
                break;
        }
        BOOST_CHECK( report.getEndTime() <= time );
    }
}

BOOST_AUTO_TEST_CASE( test_stream_get_next_spike_time )
{
    boost::filesystem::path path( BBP_TESTDATA );
    path /= NEST_SPIKE_REPORT_FILE;
    brion::SpikeReport report( brion::URI( "spikes://" + path.string( )),
                               brion::MODE_READ);

    while( report.waitUntil( report.getNextSpikeTime( )))
        ;

    BOOST_CHECK( report.getEndTime() == NEST_SPIKES_END_TIME );
}

BOOST_AUTO_TEST_CASE( test_stream_get_latest_spike_time )
{
    boost::filesystem::path path( BBP_TESTDATA );
    path /= NEST_SPIKE_REPORT_FILE;
    brion::SpikeReport report( brion::URI( "spikes://" + path.string( )),
                               brion::MODE_READ);

    report.waitUntil( 0 );
    while( report.getLatestSpikeTime() != NEST_SPIKES_END_TIME )
    {
        const float time = std::max(
            0.f, nextafterf(report.getLatestSpikeTime(), -INFINITY));
        // This function call can never block, so this loop is actually
        // making active wait
        BOOST_CHECK( report.waitUntil( time, 0 ));
    }
    // The next call will reach the end of the report
    BOOST_CHECK( !report.waitUntil( report.getLatestSpikeTime( )));
    BOOST_CHECK_EQUAL( report.getEndTime(), NEST_SPIKES_END_TIME );
}

BOOST_AUTO_TEST_CASE( test_stream_clear )
{
    boost::filesystem::path path( BBP_TESTDATA );
    path /= NEST_SPIKE_REPORT_FILE;
    brion::SpikeReport report( brion::URI( "spikes://" + path.string( )),
                               brion::MODE_READ);

    size_t lines[] = { 62, 1122, 1934, 2044 };
    double timestamps[] = { 1.8, 1.9, 2.0, 2.1 };

    BOOST_CHECK( report.waitUntil( timestamps[3] ));
    BOOST_CHECK_EQUAL( report.getSpikes().size(), lines[3] );

    report.clear( timestamps[1], timestamps[2] );
    BOOST_CHECK_EQUAL( report.getStartTime(), NEST_SPIKES_START_TIME );
    BOOST_CHECK_CLOSE( report.getEndTime(), 2.1, 0.00001 );
    BOOST_CHECK_EQUAL( report.getSpikes().size(),
                       lines[3] - ( lines[1] - lines[0] )
                                - ( lines[2] - lines[1] ));

    report.clear( timestamps[2], timestamps[3] );
    BOOST_CHECK_EQUAL( report.getStartTime(), NEST_SPIKES_START_TIME );
    BOOST_CHECK_CLOSE( report.getEndTime(), 1.8, 0.00001 );
    BOOST_CHECK_EQUAL( report.getSpikes().size(), lines[0] );

    report.clear( 0, timestamps[0] );
    BOOST_CHECK_EQUAL( report.getStartTime(), UNDEFINED_TIMESTAMP );
    BOOST_CHECK_EQUAL( report.getEndTime(), UNDEFINED_TIMESTAMP );
    BOOST_CHECK( report.getSpikes().empty( ));
}
