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

#include "paths.h"
#include <brion/brion.h>

#define BOOST_TEST_MODULE Morphology
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/foreach.hpp>

#include <cstdarg>

typedef brion::Vector4f V4f; // For brevity
typedef brion::Vector2i V2i; // For brevity

#pragma clang diagnostic ignored "-Wnon-pod-varargs"

// Ellipsis promotes enums to ints, so we need to use int.
const int SOMA = brion::SECTION_SOMA;
const int AXON = brion::SECTION_AXON;
const int DENDRITE = brion::SECTION_DENDRITE;
const int APICAL_DENDRITE = brion::SECTION_APICAL_DENDRITE;

BOOST_AUTO_TEST_CASE( invalid_open )
{
    BOOST_CHECK_THROW( brion::Morphology( "/bla" ), std::runtime_error );
    BOOST_CHECK_THROW( brion::Morphology( "bla" ), std::runtime_error );

    boost::filesystem::path path( BBP_TESTDATA );
    path /= "local/README";
    BOOST_CHECK_THROW( brion::Morphology( path.string( )), std::runtime_error );
}

BOOST_AUTO_TEST_CASE( h5_invalid_open )
{
    boost::filesystem::path path( BBP_TESTDATA );
    path /= "local/simulations/may17_2011/Control/voltage.h5";
    BOOST_CHECK_THROW( brion::Morphology( path.string( )), std::runtime_error );
}

BOOST_AUTO_TEST_CASE( h5_illegal_write )
{
    boost::filesystem::path path( BBP_TESTDATA );
    path /= "local/morphologies/01.07.08/h5/R-C010306G.h5";

    brion::Morphology morphology( path.string( ));
    BOOST_CHECK_THROW( morphology.writePoints( brion::Vector4fs(),
                                               brion::MORPHOLOGY_RAW ),
                       std::runtime_error );
    BOOST_CHECK_THROW( morphology.writeSections( brion::Vector2is(),
                                                 brion::MORPHOLOGY_RAW ),
                       std::runtime_error );
    BOOST_CHECK_THROW( morphology.writeSectionTypes( brion::SectionTypes( )),
                       std::runtime_error );
    BOOST_CHECK_THROW( morphology.writeApicals( brion::Vector2is( )),
                       std::runtime_error );
    BOOST_CHECK_THROW( morphology.flush(), std::runtime_error );
}

BOOST_AUTO_TEST_CASE( h5_overwrite )
{
    const std::string file( "overwritetest.h5" );

    boost::filesystem::remove( file );
    BOOST_CHECK_NO_THROW( brion::Morphology( file,
                                             brion::MORPHOLOGY_VERSION_H5_2,
                                             false ));
    BOOST_CHECK_THROW( brion::Morphology( file, brion::MORPHOLOGY_VERSION_H5_2,
                                          false ), std::runtime_error );
    BOOST_CHECK_NO_THROW( brion::Morphology( file,
                                             brion::MORPHOLOGY_VERSION_H5_2,
                                             true ));
    boost::filesystem::remove( file );

    BOOST_CHECK_NO_THROW( brion::Morphology( file,
                                             brion::MORPHOLOGY_VERSION_H5_2,
                                             true ));
    boost::filesystem::remove( file );
}

BOOST_AUTO_TEST_CASE( h5_read_v1 )
{
    boost::filesystem::path path( BBP_TESTDATA );
    path /= "local/morphologies/01.07.08/h5/R-C010306G.h5";

    const brion::Morphology morphology( path.string( ));

    brion::Vector4fsPtr points = morphology.readPoints( brion::MORPHOLOGY_RAW );
    BOOST_CHECK_EQUAL( points->size(), 3272 );
    BOOST_CHECK_CLOSE( (*points)[0].x(), -9.0625f, .000001f );
    BOOST_CHECK_CLOSE( (*points)[0].y(), -4.97781f, .0001f );
    BOOST_CHECK_CLOSE( (*points)[0].z(), 0.f, .000001f );
    BOOST_CHECK_CLOSE( (*points)[0].w(), 0.37f, .000001f );

    brion::Vector2isPtr sections =
                               morphology.readSections( brion::MORPHOLOGY_RAW );
    BOOST_CHECK_EQUAL( sections->size(), 138 );
    BOOST_CHECK_EQUAL( (*sections)[0].x(),  0 );
    BOOST_CHECK_EQUAL( (*sections)[0].y(), -1 );
    BOOST_CHECK_EQUAL( (*sections)[5].x(), 85 );
    BOOST_CHECK_EQUAL( (*sections)[5].y(),  4 );

    brion::SectionTypesPtr types = morphology.readSectionTypes();
    BOOST_CHECK_EQUAL( types->size(), 138 );
    BOOST_CHECK_EQUAL( (*types)[0],  1 );
    BOOST_CHECK_EQUAL( (*types)[5],  2 );
}

BOOST_AUTO_TEST_CASE( h5_write_v1 )
{
    boost::filesystem::path path( BBP_TESTDATA );
    path /= "local/morphologies/01.07.08/h5/R-C010306G.h5";

    const brion::Morphology source( path.string( ));

    brion::Vector4fsPtr points = source.readPoints( brion::MORPHOLOGY_RAW );
    brion::Vector2isPtr sections = source.readSections( brion::MORPHOLOGY_RAW );
    brion::SectionTypesPtr types = source.readSectionTypes();

    {
        brion::Morphology a( "testv1.h5",
                             brion::MORPHOLOGY_VERSION_H5_1, true );
        a.writePoints( *points, brion::MORPHOLOGY_UNDEFINED );
        a.writeSections( *sections, brion::MORPHOLOGY_UNDEFINED );
        a.writeSectionTypes( *types );
    }

    const brion::Morphology source2( "testv1.h5" );
    brion::Vector4fsPtr points2 = source2.readPoints( brion::MORPHOLOGY_RAW );
    brion::Vector2isPtr sections2 =
                                  source2.readSections( brion::MORPHOLOGY_RAW );
    brion::SectionTypesPtr types2 = source2.readSectionTypes();

    BOOST_CHECK( *points == *points2 );
    BOOST_CHECK( *sections == *sections2 );
    BOOST_CHECK( *types == *types2 );
}

BOOST_AUTO_TEST_CASE( h5_read_v2 )
{
    boost::filesystem::path path( BBP_TESTDATA );
    path /= "local/morphologies/14.07.10_repaired/v2/C010398B-P2.h5";

    const brion::Morphology morphology( path.string( ));

    brion::Vector4fsPtr points =
                            morphology.readPoints( brion::MORPHOLOGY_REPAIRED );
    BOOST_CHECK_EQUAL( points->size(), 1499 );
    BOOST_CHECK_CLOSE( (*points)[0].x(), 5.335999965667725f, .000001f );
    BOOST_CHECK_CLOSE( (*points)[0].y(), 2.702667474746704f, .000001f );
    BOOST_CHECK_CLOSE( (*points)[0].z(), -1.1733332872390747f, .000001f );
    BOOST_CHECK_CLOSE( (*points)[0].w(), 0.f, .000001f );

    brion::Vector2isPtr sections =
                          morphology.readSections( brion::MORPHOLOGY_REPAIRED );
    BOOST_CHECK_EQUAL( sections->size(), 78 );
    BOOST_CHECK_EQUAL( (*sections)[0].x(),  0 );
    BOOST_CHECK_EQUAL( (*sections)[0].y(), -1 );
    BOOST_CHECK_EQUAL( (*sections)[5].x(), 49 );
    BOOST_CHECK_EQUAL( (*sections)[5].y(),  4 );

    brion::SectionTypesPtr types = morphology.readSectionTypes();
    BOOST_CHECK_EQUAL( types->size(), 78 );
    BOOST_CHECK_EQUAL( (*types)[0],  1 );
    BOOST_CHECK_EQUAL( (*types)[5],  2 );

    brion::Vector2isPtr apicals = morphology.readApicals();
    BOOST_CHECK_EQUAL( apicals->size(), 1 );
    BOOST_CHECK_EQUAL( (*apicals)[0].x(), 67 );
    BOOST_CHECK_EQUAL( (*apicals)[0].y(), 76 );
}

BOOST_AUTO_TEST_CASE( h5_write_v2 )
{
    boost::filesystem::path path( BBP_TESTDATA );
    path /= "local/morphologies/14.07.10_repaired/v2/C010398B-P2.h5";

    const brion::Morphology source( path.string( ));

    brion::Vector4fsPtr points = source.readPoints( brion::MORPHOLOGY_REPAIRED);
    brion::Vector2isPtr sections =
                              source.readSections( brion::MORPHOLOGY_REPAIRED );
    brion::SectionTypesPtr types = source.readSectionTypes();
    brion::Vector2isPtr apicals = source.readApicals();

    {   // undefined should auto-select h5 v2
        brion::Morphology a( "testv2.h5", brion::MORPHOLOGY_VERSION_UNDEFINED,
                             true );
        a.writePoints( *points, brion::MORPHOLOGY_REPAIRED );
        a.writeSections( *sections, brion::MORPHOLOGY_REPAIRED );
        a.writeSectionTypes( *types );
        a.writeApicals( *apicals );
    }

    const brion::Morphology source2( "testv2.h5" );
    brion::Vector4fsPtr points2 =
                               source2.readPoints( brion::MORPHOLOGY_REPAIRED );
    brion::Vector2isPtr sections2 =
                             source2.readSections( brion::MORPHOLOGY_REPAIRED );
    brion::SectionTypesPtr types2 = source2.readSectionTypes();
    brion::Vector2isPtr apicals2 = source2.readApicals();

    BOOST_CHECK_EQUAL( source2.getVersion(), brion::MORPHOLOGY_VERSION_H5_2 );
    BOOST_CHECK( *points == *points2 );
    BOOST_CHECK( *sections == *sections2 );
    BOOST_CHECK( *types == *types2 );
    BOOST_CHECK( *apicals == *apicals2 );
}

BOOST_AUTO_TEST_CASE( swc_invalid_open )
{
    BOOST_CHECK_THROW( brion::Morphology( "not_found.swc"),
                       std::runtime_error );
    boost::filesystem::path path( BRION_TESTDATA );
    path /= "swc/bad_syntax.swc";
    try
    {
        brion::Morphology( path.string( ));
        BOOST_CHECK( false );
    }
    catch ( std::runtime_error& error )
    {
        BOOST_CHECK( std::string(error.what()).find( "line 6" ) !=
                     std::string::npos );
    }
}

template<typename T>
std::ostream& operator<<( std::ostream& out, std::vector< T > &list )
{
    BOOST_FOREACH( T i, list)
        out << i << ' ';
    out << std::endl;
    return out;
}

std::ostream& operator<<( std::ostream& out, std::vector< brion::SectionType > &list )
{
    BOOST_FOREACH( brion::SectionType i, list)
        out << (int)i << ' ';
    out << std::endl;
    return out;
}

template< typename T >
struct VaArgsType
{
    typedef T type;
};

template<>
struct VaArgsType< brion::SectionType >
{
    typedef int type;
};

template< typename T >
void verifyArray( const std::vector< T >& array, const size_t length, ... )
{
    // Create the reference array
    std::vector< T > ref;
    va_list args;
    va_start( args, length );
    for( size_t i = 0; i != length; ++i )
        ref.push_back( (T)va_arg( args, typename VaArgsType< T >::type ));
    va_end( args );

    BOOST_CHECK_EQUAL_COLLECTIONS( array.begin(), array.end(),
                                   ref.begin(), ref.end( ));
}

BOOST_AUTO_TEST_CASE( swc_soma )
{
    boost::filesystem::path path( BRION_TESTDATA );
    const brion::MorphologyRepairStage stage = brion::MORPHOLOGY_REPAIRED;
    path /= "swc/soma.swc";

    const brion::Morphology source( path.string( ));
    verifyArray( *source.readPoints( stage ), 1, V4f( 0, 0, 0, 20 ));
    verifyArray( *source.readSections( stage ), 1, V2i( 0, -1 ));
    verifyArray( *source.readSectionTypes(), 1, SOMA );
}

BOOST_AUTO_TEST_CASE( swc_soma_ring )
{
    boost::filesystem::path path( BRION_TESTDATA );
    const brion::MorphologyRepairStage stage = brion::MORPHOLOGY_REPAIRED;
    path /= "swc/soma_ring.swc";

    const brion::Morphology source( path.string( ));
    verifyArray( *source.readPoints( stage ), 5, V4f( 0, 0, 0, 20 ),
                 V4f( 0, 0, 1, 20 ), V4f( 0, 1, 0, 20 ), V4f( 0, 1, 1, 20 ),
                 V4f( 1, 0, 0, 20 ));
    verifyArray( *source.readSections( stage ), 1, V2i( 0, -1 ));
    verifyArray( *source.readSectionTypes(), 1, SOMA );
}

BOOST_AUTO_TEST_CASE( swc_no_soma )
{
    boost::filesystem::path path( BRION_TESTDATA );
    path /= "swc/no_soma.swc";

    BOOST_CHECK_THROW( brion::Morphology( path.string( )), std::runtime_error );
}

BOOST_AUTO_TEST_CASE( swc_two_somas )
{
    boost::filesystem::path path( BRION_TESTDATA );
    path /= "swc/two_somas.swc";

    BOOST_CHECK_THROW( brion::Morphology( path.string( )), std::runtime_error );
}

BOOST_AUTO_TEST_CASE( swc_single_section )
{
    boost::filesystem::path path( BRION_TESTDATA );
    const brion::MorphologyRepairStage stage = brion::MORPHOLOGY_REPAIRED;
    path /= "swc/single_section.swc";

    const brion::Morphology source( path.string( ));

    verifyArray( *source.readPoints( stage ), 5, V4f( 0, 0, 0, 20 ),
                 V4f( 0, 0, 1, 4 ), V4f( 0, 0, 2, 4 ), V4f( 0, 0, 3, 4 ),
                 V4f( 0, 0, 4, 4 ));
    verifyArray( *source.readSections( stage ), 2, V2i( 0, -1 ), V2i( 1, 0 ));
    verifyArray( *source.readSectionTypes(), 2, SOMA, AXON );
}

BOOST_AUTO_TEST_CASE( swc_single_section_unordered )
{
    boost::filesystem::path path( BRION_TESTDATA );
    const brion::MorphologyRepairStage stage = brion::MORPHOLOGY_REPAIRED;
    path /= "swc/single_section_unordered.swc";

    const brion::Morphology source( path.string( ));

    verifyArray( *source.readPoints( stage ), 5, V4f( 0, 0, 0, 20 ),
                 V4f( 0, 0, 1, 4 ), V4f( 0, 0, 2, 4 ), V4f( 0, 0, 3, 4 ),
                 V4f( 0, 0, 4, 4 ));
    verifyArray( *source.readSections( stage ), 2, V2i( 0, -1 ), V2i( 1, 0 ));
    verifyArray( *source.readSectionTypes(), 2, SOMA, AXON );
}

BOOST_AUTO_TEST_CASE( swc_single_section_missing_segment )
{
    boost::filesystem::path path( BRION_TESTDATA );
    path /= "swc/single_section_missing_segment.swc";

    BOOST_CHECK_THROW( brion::Morphology( path.string( )), std::runtime_error );
}

BOOST_AUTO_TEST_CASE( swc_section_type_changes )
{
    boost::filesystem::path path( BRION_TESTDATA );
    const brion::MorphologyRepairStage stage = brion::MORPHOLOGY_REPAIRED;
    path /= "swc/section_type_changes.swc";

    const brion::Morphology source( path.string( ));

    verifyArray( *source.readPoints( stage ), 7, V4f( 0, 0, 0, 20 ),
                 V4f( 0, 0, 1, 4 ), V4f( 0, 0, 2, 4 ), V4f( 0, 0, 2, 4 ),
                 V4f( 0, 0, 3, 4 ), V4f( 0, 0, 3, 4 ), V4f( 0, 0, 4, 4 ));
    verifyArray( *source.readSections( stage ), 4,
                 V2i( 0, -1 ), V2i( 1, 0 ), V2i( 3, 1 ), V2i( 5, 2 ));
    verifyArray( *source.readSectionTypes(), 4,
                 SOMA, AXON, DENDRITE, APICAL_DENDRITE );
}

BOOST_AUTO_TEST_CASE( swc_first_order_sections )
{
    boost::filesystem::path path( BRION_TESTDATA );
    const brion::MorphologyRepairStage stage = brion::MORPHOLOGY_REPAIRED;
    path /= "swc/first_order_sections.swc";

    const brion::Morphology source( path.string( ));

    verifyArray( *source.readSections( stage ), 4,
                 V2i( 0, -1 ), V2i( 1, 0 ), V2i( 2, 0 ), V2i( 3, 0 ));
    // The tree construction algorithm reserves the order of two sections
    // compared to how they appear in the file
    verifyArray( *source.readPoints( stage ), 4, V4f( 0, 0, 0, 20 ),
                 V4f( 0, 0, 1, 4 ), V4f( 0, 0, 3, 4 ), V4f( 0, 0, 2, 4 ));
    verifyArray( *source.readSectionTypes(), 4,
                 SOMA, AXON, APICAL_DENDRITE, DENDRITE );

}

BOOST_AUTO_TEST_CASE( swc_bifurcation )
{
    boost::filesystem::path path( BRION_TESTDATA );
    const brion::MorphologyRepairStage stage = brion::MORPHOLOGY_REPAIRED;
    path /= "swc/bifurcations.swc";

    const brion::Morphology source( path.string( ));

    verifyArray( *source.readPoints( stage ), 9, V4f( 0, 0, 0, 20 ),
                 V4f( 0, 0, 2, 4 ), V4f( 0, 0, 3, 4 ), V4f( 0, 0, 3, 4 ),
                 V4f( 0, 0, 4, 4 ), V4f( 0, 0, 5, 4 ), V4f( 0, 0, 3, 4 ),
                 V4f( 0, 0, 6, 4 ), V4f( 0, 0, 7, 4 ));
    verifyArray( *source.readSections( stage ), 4,
                 V2i( 0, -1 ), V2i( 1, 0 ), V2i( 3, 1 ), V2i( 6, 1 ));
    verifyArray( *source.readSectionTypes(), 4,
                 SOMA, DENDRITE, APICAL_DENDRITE, APICAL_DENDRITE );
}

BOOST_AUTO_TEST_CASE( swc_neuron )
{
    boost::filesystem::path path( BRION_TESTDATA );
    path /= "swc/Neuron.swc";

    brion::Morphology neuron( path.string( ));
    BOOST_CHECK_EQUAL( neuron.readPoints( brion::MORPHOLOGY_REPAIRED )->size(),
                       927 );
}

