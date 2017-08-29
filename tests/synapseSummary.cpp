/* Copyright (c) 2013-2015, EPFL/Blue Brain Project
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

#include <BBP/TestDatasets.h>
#include <brion/brion.h>

#define BOOST_TEST_MODULE SynapseSummary
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/test/unit_test.hpp>
#include <lunchbox/log.h>

BOOST_AUTO_TEST_CASE(test_invalid_open)
{
    BOOST_CHECK_THROW(brion::SynapseSummary("/bla"), std::runtime_error);
    BOOST_CHECK_THROW(brion::SynapseSummary("bla"), std::runtime_error);

    boost::filesystem::path path(BBP_TESTDATA);
    path /= "local/README";
    BOOST_CHECK_THROW(brion::SynapseSummary(path.string()), std::runtime_error);

    path = BBP_TESTDATA;
    path /= "local/circuits/18.10.10_600cell/ncsFunctionalCompare/nrn.h5";
    BOOST_CHECK_THROW(brion::SynapseSummary(path.string()), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(test_invalid_read)
{
    boost::filesystem::path path(BBP_TESTDATA);
    path /=
        "local/circuits/18.10.10_600cell/ncsFunctionalCompare/nrn_summary.h5";

    brion::SynapseSummary synapseSummary(path.string());
    const brion::SynapseSummaryMatrix& data = synapseSummary.read(0);
    BOOST_CHECK_EQUAL(data.shape()[0], 0);
    BOOST_CHECK_EQUAL(data.shape()[1], 0);
}

BOOST_AUTO_TEST_CASE(test_read)
{
    boost::filesystem::path path(BBP_TESTDATA);
    path /=
        "local/circuits/18.10.10_600cell/ncsFunctionalCompare/nrn_summary.h5";

    brion::SynapseSummary synapseSummary(path.string());

    const brion::SynapseSummaryMatrix& data = synapseSummary.read(1);
    BOOST_CHECK_EQUAL(data.shape()[0], 27); // 27 synapses for GID 1
    BOOST_CHECK_EQUAL(data.shape()[1], 3);  //  3 attributes
    BOOST_CHECK_EQUAL(data[0][0], 2);
    BOOST_CHECK_EQUAL(data[0][1], 0);
    BOOST_CHECK_EQUAL(data[0][2], 3);
    BOOST_CHECK_EQUAL(data[14][0], 126);
    BOOST_CHECK_EQUAL(data[14][1], 4);
    BOOST_CHECK_EQUAL(data[14][2], 0);
}

BOOST_AUTO_TEST_CASE(test_perf)
{
    boost::filesystem::path path(BBP_TESTDATA);
    path /=
        "local/circuits/18.10.10_600cell/ncsFunctionalCompare/nrn_summary.h5";
    brion::SynapseSummary synapseSummary(path.string());

    brion::GIDSet gids;
    for (uint32_t i = 1; i <= 600; ++i)
        gids.insert(i);

    namespace bp = boost::posix_time;
    const bp::ptime startTime = bp::microsec_clock::local_time();
    for (brion::GIDSetCIter i = gids.begin(); i != gids.end(); ++i)
        synapseSummary.read(*i);
    const bp::time_duration duration =
        bp::microsec_clock::local_time() - startTime;

    LBERROR << "Reading synapse information for " << gids.size()
            << " cells took: " << duration.total_milliseconds() << " ms."
            << std::endl;
}
