/* Copyright (c) 2013-2015, EPFL/Blue Brain Project
 *                          Daniel Nachbaur <daniel.nachbaur@epfl.ch>
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

#include <brion/brion.h>
#include <BBP/TestDatasets.h>

#define BOOST_TEST_MODULE SynapseSummary
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/test/unit_test.hpp>
#include <lunchbox/log.h>

BOOST_AUTO_TEST_CASE( test_invalid_open )
{
    BOOST_CHECK_THROW( brion::SynapseSummary( "/bla" ), std::runtime_error );
    BOOST_CHECK_THROW( brion::SynapseSummary( "bla" ), std::runtime_error );

    boost::filesystem::path path( BBP_TESTDATA );
    path /= "local/README";
    BOOST_CHECK_THROW( brion::SynapseSummary( path.string( )),
                       std::runtime_error );

    path = BBP_TESTDATA;
    path /= "local/circuits/18.10.10_600cell/ncsFunctionalCompare/nrn.h5";
    BOOST_CHECK_THROW( brion::SynapseSummary( path.string( )),
                       std::runtime_error );
}

BOOST_AUTO_TEST_CASE( test_invalid_read )
{
    boost::filesystem::path path( BBP_TESTDATA );
    path /= "local/circuits/18.10.10_600cell/ncsFunctionalCompare/nrn_summary.h5";

    brion::SynapseSummary synapseSummary( path.string( ));
    const brion::SynapseSummaryMatrix& data = synapseSummary.read( 0 );
    BOOST_CHECK_EQUAL( data.shape()[0], 0 );
    BOOST_CHECK_EQUAL( data.shape()[1], 0 );
}

BOOST_AUTO_TEST_CASE( test_read )
{
    boost::filesystem::path path( BBP_TESTDATA );
    path /= "local/circuits/18.10.10_600cell/ncsFunctionalCompare/nrn_summary.h5";

    brion::SynapseSummary synapseSummary( path.string( ));

    const brion::SynapseSummaryMatrix& data = synapseSummary.read( 1 );
    std::cout << data << std::endl;
    BOOST_CHECK_EQUAL( data.shape()[0], 27 );  // 27 synapses for GID 1
    BOOST_CHECK_EQUAL( data.shape()[1], 3 );   //  3 attributes
    BOOST_CHECK_EQUAL( data[0][0], 2 );
    BOOST_CHECK_EQUAL( data[0][1], 0 );
    BOOST_CHECK_EQUAL( data[0][2], 3 );
    BOOST_CHECK_EQUAL( data[14][0], 126 );
    BOOST_CHECK_EQUAL( data[14][1], 4 );
    BOOST_CHECK_EQUAL( data[14][2], 0 );
}

BOOST_AUTO_TEST_CASE( test_perf )
{
    boost::filesystem::path path( BBP_TESTDATA );
    path /= "local/circuits/18.10.10_600cell/ncsFunctionalCompare/nrn_summary.h5";
    brion::SynapseSummary synapseSummary( path.string( ));

    brion::GIDSet gids;
    for( uint32_t i = 1; i <= 600; ++ i)
        gids.insert( i );

    namespace bp = boost::posix_time;
    const bp::ptime startTime = bp::microsec_clock::local_time();
    for( brion::GIDSetCIter i = gids.begin(); i != gids.end(); ++i )
        synapseSummary.read( *i );
    const bp::time_duration duration =
                                   bp::microsec_clock::local_time() - startTime;

    LBERROR << "Reading synapse information for " << gids.size()
            << " cells took: "  << duration.total_milliseconds() << " ms."
            << std::endl;
}
