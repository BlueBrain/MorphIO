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

#define BOOST_TEST_MODULE BlueConfig
#include <boost/filesystem/path.hpp>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE( test_invalid_open )
{
    BOOST_CHECK_THROW( brion::BlueConfig( "/bla" ), std::runtime_error );
    BOOST_CHECK_THROW( brion::BlueConfig( "bla" ), std::runtime_error );

    boost::filesystem::path path( BBP_TESTDATA );
    path /= "local/README";
    BOOST_CHECK_THROW( brion::BlueConfig( path.string( )), std::runtime_error );

    path = BBP_TESTDATA;
    path /= "local/simulations/may17_2011/Control/voltage.h5";
    BOOST_CHECK_THROW( brion::BlueConfig( path.string( )), std::runtime_error );
}

BOOST_AUTO_TEST_CASE( test_verify_loaded_data )
{
    brion::BlueConfig config( bbp::test::getBlueconfig( ));
    std::cout << config << std::endl;
    if( config.get( brion::CONFIGSECTION_RUN, "Demo", "Version" ) != "777" )
        return;

    const brion::Strings& runs =
                             config.getSectionNames( brion::CONFIGSECTION_RUN );
    const brion::Strings& stimuluss =
                        config.getSectionNames( brion::CONFIGSECTION_STIMULUS );
    const brion::Strings& stimulusInjects =
                  config.getSectionNames( brion::CONFIGSECTION_STIMULUSINJECT );
    const brion::Strings& reports =
                          config.getSectionNames( brion::CONFIGSECTION_REPORT );

    BOOST_CHECK_EQUAL( runs.size(), 1 );
    BOOST_CHECK_EQUAL( runs[0], "Demo" );

    BOOST_CHECK_EQUAL( stimuluss.size(), 2 );
    BOOST_CHECK_EQUAL( stimuluss[0], "poisson1" );
    BOOST_CHECK_EQUAL( stimuluss[1], "depolarize" );

    BOOST_CHECK_EQUAL( stimulusInjects.size(), 2 );
    BOOST_CHECK_EQUAL( stimulusInjects[0], "poisson1IntoLayer4" );
    BOOST_CHECK_EQUAL( stimulusInjects[1], "depolarizeIntoColumn" );

    BOOST_CHECK_EQUAL( reports.size(), 2 );
    BOOST_CHECK_EQUAL( reports[0], "voltage" );
    BOOST_CHECK_EQUAL( reports[1], "allCompartments" );

    const std::string prefix( BBP_TESTDATA );

    BOOST_CHECK_EQUAL( config.get( brion::CONFIGSECTION_RUN, "Demo", "Note" ),
                       "Sample configuration file." );
    BOOST_CHECK_EQUAL( config.get( brion::CONFIGSECTION_RUN, "Demo", "Date" ),
                       "17:5:11" );
    BOOST_CHECK_EQUAL( config.get( brion::CONFIGSECTION_RUN, "Demo", "Time" ),
                       "15:57:14" );
    BOOST_CHECK_EQUAL( config.get( brion::CONFIGSECTION_RUN, "Demo", "svnPath"),
                       "https://bbpteam.epfl.ch/svn/bluebrain/trunk" );
    BOOST_CHECK_EQUAL( config.get( brion::CONFIGSECTION_RUN, "Demo", "Version"),
                       "777" );
    BOOST_CHECK_EQUAL( config.get( brion::CONFIGSECTION_RUN, "Demo", "Prefix" ),
                       prefix + "/local/release/21.02.11" );
    BOOST_CHECK_EQUAL( config.get( brion::CONFIGSECTION_RUN, "Demo",
                                   "MorphologyPath" ),
                       prefix + "/local/morphologies/01.07.08" );
    BOOST_CHECK_EQUAL( config.get( brion::CONFIGSECTION_RUN, "Demo",
                                   "METypePath" ),
                       prefix + "/local/release/07.09.09/metypes" );
    BOOST_CHECK_EQUAL( config.get( brion::CONFIGSECTION_RUN, "Demo",
                                   "CircuitPath" ),
                       prefix + "/local/circuits/18.10.10_600cell" );
    BOOST_CHECK_EQUAL( config.get( brion::CONFIGSECTION_RUN, "Demo", "nrnPath"),
             prefix + "/local/circuits/18.10.10_600cell/ncsFunctionalCompare" );
    BOOST_CHECK_EQUAL( config.get( brion::CONFIGSECTION_RUN, "Demo",
                                   "MeshPath" ),
                       prefix + "/local/meshes/08.05.09" );
    BOOST_CHECK_EQUAL( config.get( brion::CONFIGSECTION_RUN, "Demo",
                                   "CurrentDir" ),
                       prefix + "/local/simulations/may17_2011" );
    BOOST_CHECK_EQUAL( config.get( brion::CONFIGSECTION_RUN, "Demo",
                                   "OutputRoot" ),
                       prefix + "/local/simulations/may17_2011/Control" );
    BOOST_CHECK_EQUAL( config.get( brion::CONFIGSECTION_RUN, "Demo",
                                   "TargetFile" ),
                 prefix + "/local/simulations/may17_2011/Control/user.target" );
    BOOST_CHECK_EQUAL( config.get( brion::CONFIGSECTION_RUN, "Demo",
                                   "Duration" ),
                       "10" );
    BOOST_CHECK_EQUAL( config.get( brion::CONFIGSECTION_RUN, "Demo", "Dt" ),
                       "0.025" );
    BOOST_CHECK_EQUAL( config.get< float >( brion::CONFIGSECTION_RUN, "Demo",
                                            "Dt" ),
                       0.025f );
    BOOST_CHECK_EQUAL( config.get( brion::CONFIGSECTION_RUN, "Demo",
                                   "CircuitTarget"),
                       "Column" );

    BOOST_CHECK_EQUAL( config.get( brion::CONFIGSECTION_RUN, "err",
                                   "CircuitTarget"),
                       "" );
    BOOST_CHECK_EQUAL( config.get( brion::CONFIGSECTION_RUN, "Demo", "err" ),
                       "" );

    BOOST_CHECK_EQUAL( config.get( brion::CONFIGSECTION_REPORT, "voltage",
                                   "Target" ), "Column" );
    BOOST_CHECK_EQUAL( config.get( brion::CONFIGSECTION_REPORT, "voltage",
                                   "Type" ), "compartment" );
    BOOST_CHECK_EQUAL( config.get( brion::CONFIGSECTION_REPORT, "voltage",
                                   "ReportOn" ), "v" );
    BOOST_CHECK_EQUAL( config.get( brion::CONFIGSECTION_REPORT, "voltage",
                                   "Unit" ), "mV" );
    BOOST_CHECK_EQUAL( config.get( brion::CONFIGSECTION_REPORT, "voltage",
                                   "Format" ), "Bin" );
    BOOST_CHECK_EQUAL( config.get<float>( brion::CONFIGSECTION_REPORT,
                                          "voltage", "Dt" ), 0.1f );
    BOOST_CHECK_EQUAL( config.get<float>( brion::CONFIGSECTION_REPORT,
                                          "voltage", "StartTime" ),  0.f );
    BOOST_CHECK_EQUAL( config.get<float>( brion::CONFIGSECTION_REPORT,
                                          "voltage", "EndTime" ), 99.f );
}

BOOST_AUTO_TEST_CASE( semantic_api )
{
    const std::string prefix( BBP_TESTDATA );
    const brion::BlueConfig config( bbp::test::getBlueconfig( ));
    const std::string root = config.get( brion::CONFIGSECTION_RUN, "Demo",
                                         "OutputRoot" );

    BOOST_CHECK_EQUAL( config.getCircuitSource(),
        brion::URI( prefix + "/local/circuits/18.10.10_600cell/circuit.mvd2" ));
    BOOST_CHECK_EQUAL( config.getSynapseSource(),
        brion::URI( prefix + "/local/circuits/18.10.10_600cell/ncsFunctionalCompare" ));
    BOOST_CHECK_EQUAL( config.getMorphologySource(),
        brion::URI( prefix + "/local/morphologies/01.07.08/h5" ));

    BOOST_CHECK_EQUAL( config.getReportSource( "unknown"), brion::URI( ));
    const brion::URI allCompartments =
        config.getReportSource( "allCompartments" );
    BOOST_CHECK_EQUAL( allCompartments.getScheme(), "file" );
    BOOST_CHECK_EQUAL( allCompartments.getPath(),
                       root + "/allCompartments.bbp" );

    const brion::URI spikes = config.getSpikeSource();
    BOOST_CHECK_EQUAL( spikes.getScheme(), "file" );
    BOOST_CHECK_EQUAL( spikes.getPath(), root + "/out.dat" );

    BOOST_CHECK_EQUAL( config.getCircuitTarget(), "Column" );
    BOOST_CHECK_EQUAL( config.getTimestep( ), 0.025f );
}

BOOST_AUTO_TEST_CASE( parse_target )
{
    const brion::BlueConfig config( bbp::test::getBlueconfig( ));
    BOOST_CHECK_THROW( config.parseTarget( "" ), std::runtime_error );
    const brion::GIDSet columnTarget = config.parseTarget( "Column" );
    BOOST_CHECK( columnTarget.size( ));

    const brion::GIDSet fromUserTarget = config.parseTarget( "AllL5CSPC" );
    BOOST_CHECK( fromUserTarget.size( ));
    const brion::GIDSet fromStartTarget = config.parseTarget( "L5CSPC" );
    BOOST_CHECK( fromStartTarget.size( ));
    BOOST_CHECK( fromStartTarget == fromUserTarget );

    BOOST_CHECK_THROW( config.parseTarget( "unexistent" ), std::runtime_error );
}
