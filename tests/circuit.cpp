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

#define BOOST_TEST_MODULE Circuit
#include <boost/filesystem/path.hpp>
#include <boost/test/unit_test.hpp>
#include <lunchbox/bitOperation.h>

namespace
{
std::string getValue( const brion::NeuronMatrix& data, const size_t idx,
                       const uint32_t attr )
{
    return data[idx][lunchbox::getIndexOfLastBit( attr )];
}
}

BOOST_AUTO_TEST_CASE( test_invalid_open )
{
    BOOST_CHECK_THROW( brion::Circuit( "/bla" ), std::runtime_error );
    BOOST_CHECK_THROW( brion::Circuit( "bla" ), std::runtime_error );

    boost::filesystem::path path( BBP_TESTDATA );
    path /= "local/README";
    BOOST_CHECK_THROW( brion::Circuit( path.string( )), std::runtime_error );

    path = BBP_TESTDATA;
    path /= "local/simulations/may17_2011/Control/voltage.h5";
    BOOST_CHECK_THROW( brion::Circuit( path.string( )), std::runtime_error );
}

BOOST_AUTO_TEST_CASE(test_all_attributes)
{
    boost::filesystem::path path( BBP_TESTDATA );
    path /= "local/circuits/circuit.mvd2";

    brion::Circuit circuit( path.string( ));
    BOOST_CHECK_EQUAL( circuit.getNumNeurons(), 10 );

    const brion::NeuronMatrix& data = circuit.get( brion::GIDSet(),
                                                 brion::NEURON_ALL_ATTRIBUTES );
    std::cout << data << std::endl;

    BOOST_CHECK_EQUAL( data.shape()[0], 10 );   // 10 neurons
    BOOST_CHECK_EQUAL( data.shape()[1], brion::NEURON_ALL );
    BOOST_CHECK_EQUAL( getValue( data, 0, brion::NEURON_MORPHOLOGY_NAME ),
                       "R-BJM141005C2_B_cor" );
    BOOST_CHECK_EQUAL( getValue( data, 1, brion::NEURON_COLUMN_GID ), "0" );
    BOOST_CHECK_EQUAL( getValue( data, 6, brion::NEURON_MTYPE ), "17" );
    BOOST_CHECK_EQUAL( getValue( data, 7, brion::NEURON_POSITION_Y ),
                       "399.305168" );
}

BOOST_AUTO_TEST_CASE(test_some_attributes)
{
    boost::filesystem::path path( BBP_TESTDATA );
    path /= "local/circuits/circuit.mvd2";

    brion::Circuit circuit( path.string( ));
    BOOST_CHECK_EQUAL( circuit.getNumNeurons(), 10 );

    brion::GIDSet gids;
    gids.insert( 4 );
    gids.insert( 6 );
    const brion::NeuronMatrix& data = circuit.get( gids,
                          brion::NEURON_ETYPE | brion::NEURON_MORPHOLOGY_NAME );
    std::cout << data << std::endl;

    BOOST_CHECK_EQUAL( data.shape()[0], 2 );   // 2 neurons
    BOOST_CHECK_EQUAL( data.shape()[1], 2 );   // 2 attributes
    BOOST_CHECK_EQUAL( data[0][0], "L2PC32_2" );
    BOOST_CHECK_EQUAL( data[0][1], "0" );
    BOOST_CHECK_EQUAL( data[1][0], "R-C010600A2" );
    BOOST_CHECK_EQUAL( data[1][1], "3" );
}

BOOST_AUTO_TEST_CASE(test_types)
{
    boost::filesystem::path path( BBP_TESTDATA );
    path /= "local/circuits/18.10.10_600cell/circuit.mvd2";

    brion::Circuit circuit( path.string( ));
    BOOST_CHECK_EQUAL( circuit.getNumNeurons(), 600 );

    const brion::Strings& mtypes = circuit.getTypes( brion::NEURONCLASS_MTYPE );
    BOOST_CHECK_EQUAL( mtypes.size(), 22 );
    BOOST_CHECK_EQUAL( mtypes[0], "AHC" );
    BOOST_CHECK_EQUAL( mtypes[1], "NGC" );
    BOOST_CHECK_EQUAL( mtypes[2], "ADC" );
    BOOST_CHECK_EQUAL( mtypes[15], "L4SP" );
    BOOST_CHECK_EQUAL( mtypes[21], "L6FFPC" );

    const brion::Strings& mclasses =
                        circuit.getTypes( brion::NEURONCLASS_MORPHOLOGY_CLASS );
    BOOST_CHECK_EQUAL( mclasses.size(), 22 );
    BOOST_CHECK_EQUAL( mclasses[0], "INT" );
    BOOST_CHECK_EQUAL( mclasses[1], "INT" );
    BOOST_CHECK_EQUAL( mclasses[4], "PYR" );
    BOOST_CHECK_EQUAL( mclasses[21], "PYR" );

    const brion::Strings& fclasses =
                          circuit.getTypes( brion::NEURONCLASS_FUNCTION_CLASS );
    BOOST_CHECK_EQUAL( fclasses.size(), 22 );
    BOOST_CHECK_EQUAL( fclasses[0], "INH" );
    BOOST_CHECK_EQUAL( fclasses[1], "INH" );
    BOOST_CHECK_EQUAL( fclasses[4], "EXC" );
    BOOST_CHECK_EQUAL( fclasses[21], "EXC" );

    const brion::Strings& etypes =
                                   circuit.getTypes( brion::NEURONCLASS_ETYPE );
    BOOST_CHECK_EQUAL( etypes.size(), 8 );
    BOOST_CHECK_EQUAL( etypes[0], "cADint" );
    BOOST_CHECK_EQUAL( etypes[1], "cFS" );
    BOOST_CHECK_EQUAL( etypes[2], "dFS" );
    BOOST_CHECK_EQUAL( etypes[3], "cNA" );
    BOOST_CHECK_EQUAL( etypes[4], "cADpyr" );
    BOOST_CHECK_EQUAL( etypes[5], "bNA" );
    BOOST_CHECK_EQUAL( etypes[6], "bAD" );
    BOOST_CHECK_EQUAL( etypes[7], "cST" );
}
