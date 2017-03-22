
/* Copyright (c) 2013-2015, EPFL/Blue Brain Project
 *                          Daniel Nachbaur <daniel.nachbaur@epfl.ch>
 *                          Stefan.Eilemann@epfl.ch
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

#define BOOST_TEST_MODULE Synapse
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/test/unit_test.hpp>
#include <lunchbox/scopedMutex.h>
#include <lunchbox/spinLock.h>

// Some helper macros to make Boost::Test thread safe
// http://thread.gmane.org/gmane.comp.lib.boost.devel/123662/focus=123678
lunchbox::SpinLock testLock;
#define TS_BOOST_CHECK_EQUAL(L, R)                 \
    {                                              \
        lunchbox::ScopedFastWrite mutex(testLock); \
        BOOST_CHECK_EQUAL((L), (R));               \
    }
#define TS_BOOST_CHECK_GT(L, R)                    \
    {                                              \
        lunchbox::ScopedFastWrite mutex(testLock); \
        BOOST_CHECK_GT((L), (R));                  \
    }

BOOST_AUTO_TEST_CASE(invalid_open)
{
    BOOST_CHECK_THROW(brion::Synapse("/bla"), std::runtime_error);
    BOOST_CHECK_THROW(brion::Synapse("bla"), std::runtime_error);

    boost::filesystem::path path(BBP_TESTDATA);
    path /= "CMakeLists.txt";
    BOOST_CHECK_THROW(brion::Synapse(path.string()), std::runtime_error);

    path = BBP_TESTDATA;
    path /= "circuitBuilding_1000neurons/Functionalizer_output/nrn_summary.h5";
    BOOST_CHECK_THROW(brion::Synapse(path.string()), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(invalid_read)
{
    boost::filesystem::path path(BBP_TESTDATA);
    path /= "circuitBuilding_1000neurons/Functionalizer_output/nrn.h5";

    const brion::Synapse synapseFile(path.string());
    const brion::SynapseMatrix& data =
        synapseFile.read(0, brion::SYNAPSE_ALL_ATTRIBUTES);
    BOOST_CHECK_EQUAL(data.shape()[0], 0);
    BOOST_CHECK_EQUAL(data.shape()[1], 0);
}

BOOST_AUTO_TEST_CASE(read_attributes)
{
    boost::filesystem::path path(BBP_TESTDATA);
    path /= "circuitBuilding_1000neurons/Functionalizer_output/nrn.h5";

    const brion::Synapse synapseFile(path.string());
    const brion::SynapseMatrix& empty =
        synapseFile.read(1, brion::SYNAPSE_NO_ATTRIBUTES);
    BOOST_CHECK_EQUAL(empty.shape()[0], 0);
    BOOST_CHECK_EQUAL(empty.shape()[1], 0);

    const brion::SynapseMatrix& data =
        synapseFile.read(1, brion::SYNAPSE_ALL_ATTRIBUTES);
    BOOST_CHECK_EQUAL(data.shape()[0], 77); // 77 synapses for GID 1
    BOOST_CHECK_EQUAL(data.shape()[1], 19); // 19 (==all) synapse attributes
    BOOST_CHECK_EQUAL(data[0][0], 10);
    BOOST_CHECK_EQUAL(data[1][0], 10);
    BOOST_CHECK_EQUAL(data[2][0], 10);
    BOOST_CHECK_EQUAL(data[3][0], 10);
    BOOST_CHECK_EQUAL(data[4][0], 10);
    BOOST_CHECK_EQUAL(data[5][0], 107);
    BOOST_CHECK_EQUAL(data[6][0], 107);

    const brion::SynapseMatrix& data2 =
        synapseFile.read(4, brion::SYNAPSE_DELAY);
    BOOST_CHECK_EQUAL(data2.shape()[0], 41); // 41 synapses for GID 4
    BOOST_CHECK_EQUAL(data2.shape()[1], 1);  // 1  synapse attributes
    BOOST_CHECK_CLOSE(data2[0][0], 1.46838176f, .0003f);
    BOOST_CHECK_CLOSE(data2[4][0], 1.46865427f, .0003f);
    BOOST_CHECK_CLOSE(data2[9][0], 2.21976233f, .0003f);
}

BOOST_AUTO_TEST_CASE(parallel_read)
{
    boost::filesystem::path path(BBP_TESTDATA);
    path /= "circuitBuilding_1000neurons/Functionalizer_output/nrn.h5";

    const brion::Synapse synapseFile(path.string());

    brion::uint32_ts connectedNeurons(100);
    brion::GIDSet gids;
    for (uint32_t i = 1; i <= 100; ++i)
    {
        const brion::SynapseMatrix& data =
            synapseFile.read(i, brion::SYNAPSE_CONNECTED_NEURON);
        connectedNeurons[i - 1] = data[0][0];
        gids.insert(i);
    }

#pragma omp parallel
    for (uint32_t i = 1; i <= 100; ++i)
    {
        const brion::SynapseMatrix& data =
            synapseFile.read(i, brion::SYNAPSE_ALL_ATTRIBUTES);
        TS_BOOST_CHECK_EQUAL(connectedNeurons[i - 1], data[0][0]);
        TS_BOOST_CHECK_GT(synapseFile.getNumSynapses(gids), 0);
    }
}

BOOST_AUTO_TEST_CASE(read_positions)
{
    boost::filesystem::path path(BBP_TESTDATA);
    path /=
        "circuitBuilding_1000neurons/Functionalizer_output/nrn_positions.h5";

    const brion::Synapse synapseFile(path.string());
    const brion::SynapseMatrix& empty =
        synapseFile.read(1, brion::SYNAPSE_POSITION_NO_ATTRIBUTES);
    BOOST_CHECK_EQUAL(empty.shape()[0], 0);
    BOOST_CHECK_EQUAL(empty.shape()[1], 0);

    const brion::SynapseMatrix& data =
        synapseFile.read(1, brion::SYNAPSE_POSITION);
    BOOST_CHECK_EQUAL(data.shape()[0], 77); // 77 synapses for GID 1
    BOOST_CHECK_EQUAL(data.shape()[1], 13); // 19 (==all) synapse attributes
    BOOST_CHECK_EQUAL(data[0][0], 10);
    BOOST_CHECK_CLOSE(data[0][1], 3.79281569f, .0003f);
    BOOST_CHECK_CLOSE(data[0][2], 1947.05054f, .0003f);
    BOOST_CHECK_CLOSE(data[0][3], 9.21417809f, .0003f);
    BOOST_CHECK_CLOSE(data[0][4], 3.60336041f, .0003f);
    BOOST_CHECK_CLOSE(data[0][5], 1947.14514f, .0003f);
    BOOST_CHECK_CLOSE(data[0][6], 9.20550251f, .0003f);

    const brion::SynapseMatrix& data2 =
        synapseFile.read(4, brion::SYNAPSE_POSTSYNAPTIC_SURFACE_Y);
    BOOST_CHECK_EQUAL(data2.shape()[0], 41); // 41 synapses for GID 4
    BOOST_CHECK_EQUAL(data2.shape()[1], 1);  // 1  synapse attribute
    BOOST_CHECK_CLOSE(data2[0][0], 2029.24304f, .0003f);
    BOOST_CHECK_CLOSE(data2[4][0], 2003.80627f, .0003f);
    BOOST_CHECK_CLOSE(data2[9][0], 2001.01599f, .0003f);
}

BOOST_AUTO_TEST_CASE(getNumSynapses)
{
    boost::filesystem::path path(BBP_TESTDATA);
    path /= "circuitBuilding_1000neurons/Functionalizer_output/nrn.h5";

    const brion::Synapse synapseFile(path.string());
    brion::GIDSet gids;

    size_t numSynapses = synapseFile.getNumSynapses(gids);
    BOOST_CHECK_EQUAL(numSynapses, 0);

    uint32_t i = 1;
    for (; i <= 10; ++i)
        gids.insert(i);

    numSynapses = synapseFile.getNumSynapses(gids);
    BOOST_CHECK_EQUAL(numSynapses, 648);

    for (; i <= 20; ++i)
        gids.insert(i);

    numSynapses = synapseFile.getNumSynapses(gids);
    BOOST_CHECK_EQUAL(numSynapses, 1172);
}

BOOST_AUTO_TEST_CASE(perf)
{
    boost::filesystem::path path(BBP_TESTDATA);
    path /= "circuitBuilding_1000neurons/Functionalizer_output/nrn.h5";
    const brion::Synapse synapseFile(path.string());

    brion::GIDSet gids;
    for (uint32_t i = 1; i <= 1000; ++i)
        gids.insert(i);

    namespace bp = boost::posix_time;
    bp::ptime startTime = bp::microsec_clock::local_time();
    const size_t numSynapses = synapseFile.getNumSynapses(gids);
    bp::time_duration duration = bp::microsec_clock::local_time() - startTime;
    LBERROR << "Reading synapse count for " << gids.size()
            << " cells took: " << duration.total_milliseconds() << " ms."
            << std::endl;

    startTime = bp::microsec_clock::local_time();
    for (brion::GIDSetCIter i = gids.begin(); i != gids.end(); ++i)
        synapseFile.read(*i, brion::SYNAPSE_DEPRESSION);
    duration = bp::microsec_clock::local_time() - startTime;
    LBERROR << "Reading one attribute for " << numSynapses << " synapses for "
            << gids.size() << " cells took: " << duration.total_milliseconds()
            << " ms." << std::endl;

    startTime = bp::microsec_clock::local_time();
    for (brion::GIDSetCIter i = gids.begin(); i != gids.end(); ++i)
        synapseFile.read(*i, brion::SYNAPSE_ALL_ATTRIBUTES);
    duration = bp::microsec_clock::local_time() - startTime;
    LBERROR << "Reading all attributes for " << numSynapses << " synapses for "
            << gids.size() << " cells took: " << duration.total_milliseconds()
            << " ms." << std::endl;

    startTime = bp::microsec_clock::local_time();
    for (brion::GIDSetCIter i = gids.begin(); i != gids.end(); ++i)
    {
        synapseFile.read(*i, brion::SynapseAttributes(
                                 brion::SYNAPSE_ALL_ATTRIBUTES &
                                 ~brion::SYNAPSE_DEPRESSION));
    }
    duration = bp::microsec_clock::local_time() - startTime;
    LBERROR << "Reading almost all attributes for " << numSynapses
            << " synapses for " << gids.size()
            << " cells took: " << duration.total_milliseconds() << " ms."
            << std::endl;
}

BOOST_AUTO_TEST_CASE(read_unmerged)
{
    boost::filesystem::path path(BBP_TESTDATA);
    path /= "local/unmergedSynapses/nrn.h5";

    const brion::Synapse synapseFile(path.string());
    const brion::SynapseMatrix& data =
        synapseFile.read(1, brion::SYNAPSE_ALL_ATTRIBUTES);
    BOOST_CHECK_EQUAL(data.shape()[0], 376); // synapses for GID 1
    BOOST_CHECK_EQUAL(data.shape()[1], 19);  // 19 (==all) synapse attributes
    BOOST_CHECK_EQUAL(data[0][0], 6);
    BOOST_CHECK_EQUAL(data[1][0], 6);
    BOOST_CHECK_EQUAL(data[2][0], 11);
    BOOST_CHECK_EQUAL(data[3][0], 11);
    BOOST_CHECK_EQUAL(data[4][0], 12);
    BOOST_CHECK_EQUAL(data[5][0], 12);
    BOOST_CHECK_EQUAL(data[6][0], 20);

    brion::GIDSet gids;
    gids.insert(1);
    size_t numSynapses = synapseFile.getNumSynapses(gids);
    BOOST_CHECK_EQUAL(numSynapses, 376);

    for (uint32_t i = 2; i <= 10; ++i)
        gids.insert(i);

    numSynapses = synapseFile.getNumSynapses(gids);
    BOOST_CHECK_EQUAL(numSynapses, 2903);
}
