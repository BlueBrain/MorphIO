/* Copyright (c) 2014-2017, EPFL/Blue Brain Project
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
#include <lunchbox/scopedMutex.h>
#include <lunchbox/spinLock.h>

#define BOOST_TEST_MODULE ThreadHDF5
#include <boost/filesystem/path.hpp>
#include <boost/test/unit_test.hpp>

// Some helper macros to make Boost::Test thread safe
// http://thread.gmane.org/gmane.comp.lib.boost.devel/123662/focus=123678
lunchbox::SpinLock testLock;
#define TS_BOOST_CHECK_NO_THROW(L)                 \
    {                                              \
        lunchbox::ScopedFastWrite mutex(testLock); \
        BOOST_CHECK_NO_THROW((L));                 \
    }
#define TS_BOOST_CHECK(L)                          \
    {                                              \
        lunchbox::ScopedFastWrite mutex(testLock); \
        BOOST_CHECK((L));                          \
    }
#define TS_BOOST_CHECK_GT(L, R)                    \
    {                                              \
        lunchbox::ScopedFastWrite mutex(testLock); \
        BOOST_CHECK_GT((L), (R));                  \
    }

BOOST_AUTO_TEST_CASE(test_parallel_open_of_same_synapse)
{
    boost::filesystem::path path(BBP_TESTDATA);
    path /= "circuitBuilding_1000neurons/Functionalizer_output/nrn.h5";

#pragma omp parallel
    TS_BOOST_CHECK_NO_THROW(brion::Synapse(path.string()));
}

BOOST_AUTO_TEST_CASE(test_parallel_access_of_synapse)
{
    boost::filesystem::path path(BBP_TESTDATA);
    path /= "circuitBuilding_1000neurons/Functionalizer_output/nrn.h5";

    brion::GIDSet gids;
    gids.insert(1);
    gids.insert(2);

    const brion::Synapse synapse(path.string());
#pragma omp parallel
    {
        const brion::SynapseMatrix& data =
            synapse.read(1, brion::SYNAPSE_ALL_ATTRIBUTES);
        TS_BOOST_CHECK(!data.empty());
        const size_t numSynapses = synapse.getNumSynapses(gids);
        TS_BOOST_CHECK_GT(numSynapses, 0);
    }
}

BOOST_AUTO_TEST_CASE(test_parallel_open_of_same_synapse_summary)
{
    boost::filesystem::path path(BBP_TESTDATA);
    path /= "circuitBuilding_1000neurons/Functionalizer_output/nrn_summary.h5";

#pragma omp parallel
    TS_BOOST_CHECK_NO_THROW(brion::SynapseSummary(path.string()));
}

BOOST_AUTO_TEST_CASE(test_parallel_open_of_same_morphology)
{
    boost::filesystem::path path(BBP_TESTDATA);
    path /= "circuitBuilding_1000neurons/morphologies/h5/C040426.h5";

#pragma omp parallel
    TS_BOOST_CHECK_NO_THROW(brion::Morphology(path.string()));
}

BOOST_AUTO_TEST_CASE(test_parallel_acess_of_morphology)
{
    boost::filesystem::path path(BBP_TESTDATA);
    path /= "circuitBuilding_1000neurons/morphologies/h5/C040426.h5";

    const brion::Morphology morphology(path.string());
#pragma omp parallel
    {
        const brion::Vector4fsPtr points = morphology.readPoints();
        const brion::Vector2isPtr sections = morphology.readSections();
        const brion::SectionTypesPtr types = morphology.readSectionTypes();
        BOOST_CHECK(!points->empty());
        BOOST_CHECK(!sections->empty());
        BOOST_CHECK(!types->empty());
    }
}
