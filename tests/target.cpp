/* Copyright (c) 2013-2015, EPFL/Blue Brain Project
 *                          Daniel Nachbaur <daniel.nachbaur@epfl.ch>
 *                          Stefan.Eilemann@epfl.ch
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

#define BOOST_TEST_MODULE Target
#include <boost/filesystem/path.hpp>
#include <boost/test/unit_test.hpp>


BOOST_AUTO_TEST_CASE( invalid_open )
{
    BOOST_CHECK_THROW( brion::Target( "blub" ), std::runtime_error );

    boost::filesystem::path path( BBP_TESTDATA );
    path /= "local/README";
    BOOST_CHECK_THROW( brion::Target( path.string( )), std::runtime_error );
}

BOOST_AUTO_TEST_CASE( get )
{
    boost::filesystem::path path( BBP_TESTDATA );
    path /= "local/circuits/18.10.10_600cell/ncsFunctionalCompare/start.target";

    const brion::Target target( path.string( ));
    std::cout << target << std::endl;

    const brion::Strings& targets = target.getTargetNames( brion::TARGET_CELL );
    BOOST_CHECK_EQUAL( targets.size(), 46 );
    BOOST_CHECK_EQUAL( target.getTargetNames(
                                        brion::TARGET_COMPARTMENT ).size(), 0 );

    const brion::Strings& columnTarget = target.get( targets[0] );
    BOOST_CHECK_EQUAL( columnTarget.size(), 6 );
    BOOST_CHECK_EQUAL( columnTarget[0], "Layer1" );

    const brion::Strings& layer4Target = target.get( targets[4] );
    BOOST_CHECK_EQUAL( layer4Target.size(), 124 );
    BOOST_CHECK_EQUAL( layer4Target[0], "a269" );
    BOOST_CHECK_EQUAL( layer4Target[10], "a279" );
    BOOST_CHECK_EQUAL( layer4Target[42], "a311" );
}

BOOST_AUTO_TEST_CASE( parse )
{
    boost::filesystem::path path( BBP_TESTDATA );
    path /= "local/circuits/18.10.10_600cell/ncsFunctionalCompare/start.target";
    const brion::Target target( path.string( ));
    const brion::GIDSet& column = brion::Target::parse(
        brion::Targets( 1, target ), "Column" );
    BOOST_CHECK_EQUAL( column.size(), 600 );
}

BOOST_AUTO_TEST_CASE( parseBroken )
{
    boost::filesystem::path pathUser( BBP_TEST_USER_TARGET );
    boost::filesystem::path pathStart( BBP_TEST_START_TARGET );

    const brion::Target startTarget( pathUser.string( ));
    const brion::Target userTarget( pathStart.string( ));

    brion::Targets testTargets;
    testTargets.push_back( startTarget );
    testTargets.push_back( userTarget );

    const brion::GIDSet& column = brion::Target::parse(
        testTargets, "Column" );

    BOOST_CHECK_NO_THROW( brion::Target::parse(
                          testTargets, "Column" ));
    BOOST_CHECK_NO_THROW( brion::Target::parse(
                          testTargets, "EmptyColumn" ));
    BOOST_CHECK_NO_THROW( brion::Target::parse(
                          testTargets, "EmptyTarget" ));
    BOOST_CHECK_THROW( brion::Target::parse(
                       testTargets, "BrokenColumn" ),
                       std::runtime_error );

    BOOST_CHECK_EQUAL( column.size(), 1000 );
}
