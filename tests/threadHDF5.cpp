/* Copyright (c) 2014-2015, EPFL/Blue Brain Project
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
#include <lunchbox/scopedMutex.h>
#include <lunchbox/spinLock.h>

#define BOOST_TEST_MODULE ThreadHDF5
#include <boost/filesystem/path.hpp>
#include <boost/test/unit_test.hpp>

// Some helper macros to make Boost::Test thread safe
// http://thread.gmane.org/gmane.comp.lib.boost.devel/123662/focus=123678
lunchbox::SpinLock testLock;
#define TS_BOOST_CHECK_NO_THROW( L )                 \
    {                                                \
        lunchbox::ScopedFastWrite mutex( testLock ); \
        BOOST_CHECK_NO_THROW( ( L ) );               \
    }
#define TS_BOOST_CHECK( L )                 \
    {                                                \
        lunchbox::ScopedFastWrite mutex( testLock ); \
        BOOST_CHECK( ( L ) );               \
    }
#define TS_BOOST_CHECK_GT( L, R )                 \
    {                                                \
        lunchbox::ScopedFastWrite mutex( testLock ); \
        BOOST_CHECK_GT( ( L ), ( R ) );              \
    }

BOOST_AUTO_TEST_CASE( test_parallel_open_of_same_synapse )
{
    boost::filesystem::path path( BBP_TESTDATA );
    path /= "circuitBuilding_1000neurons/Functionalizer_output/nrn.h5";

#pragma omp parallel
    TS_BOOST_CHECK_NO_THROW( brion::Synapse( path.string( )));
}

BOOST_AUTO_TEST_CASE( test_parallel_access_of_synapse )
{
    boost::filesystem::path path( BBP_TESTDATA );
    path /= "circuitBuilding_1000neurons/Functionalizer_output/nrn.h5";

    brion::GIDSet gids;
    gids.insert( 1 );
    gids.insert( 2 );

    const brion::Synapse synapse( path.string( ));
#pragma omp parallel
    {
        const brion::SynapseMatrix& data = synapse.read( 1,
                                                brion::SYNAPSE_ALL_ATTRIBUTES );
        TS_BOOST_CHECK( !data.empty( ));
        const size_t numSynapses = synapse.getNumSynapses( gids );
        TS_BOOST_CHECK_GT( numSynapses, 0 );
    }
}

BOOST_AUTO_TEST_CASE( test_parallel_open_of_same_synapse_summary )
{
    boost::filesystem::path path( BBP_TESTDATA );
    path /= "circuitBuilding_1000neurons/Functionalizer_output/nrn_summary.h5";

#pragma omp parallel
    TS_BOOST_CHECK_NO_THROW( brion::SynapseSummary( path.string( )));
}

BOOST_AUTO_TEST_CASE( test_parallel_open_of_same_morphology )
{
    boost::filesystem::path path( BBP_TESTDATA );
    path /= "circuitBuilding_1000neurons/morphologies/h5/C040426.h5";

#pragma omp parallel
    TS_BOOST_CHECK_NO_THROW( brion::Morphology( path.string( )));
}

BOOST_AUTO_TEST_CASE( test_parallel_acess_of_morphology )
{
    boost::filesystem::path path( BBP_TESTDATA );
    path /= "circuitBuilding_1000neurons/morphologies/h5/C040426.h5";

    const brion::Morphology morphology( path.string( ));
#pragma omp parallel
    {
        const brion::Vector4fsPtr points =
                                 morphology.readPoints( brion::MORPHOLOGY_RAW );
        const brion::Vector2isPtr sections =
                               morphology.readSections( brion::MORPHOLOGY_RAW );
        const brion::SectionTypesPtr types = morphology.readSectionTypes();
        BOOST_CHECK( !points->empty( ));
        BOOST_CHECK( !sections->empty( ));
        BOOST_CHECK( !types->empty( ));
    }
}
