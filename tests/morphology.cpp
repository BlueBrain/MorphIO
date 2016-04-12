/* Copyright (c) 2013-2016, EPFL/Blue Brain Project
 *                          Daniel Nachbaur <daniel.nachbaur@epfl.ch>
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

#include "paths.h"
#include <brion/brion.h>
#include <brain/brain.h>

#define BOOST_TEST_MODULE Morphology
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/foreach.hpp>

#include <cstdarg>
#include <cmath>

// typdef for  brevity
typedef brion::Vector4f V4f;
typedef brion::Vector3f V3f;
typedef brion::Vector2i V2i;

namespace
{
const std::string TEST_MORPHOLOGY_FILENAME =
    std::string( BRION_TESTDATA ) + "/h5/test_neuron.h5";
const brion::URI TEST_MORPHOLOGY_URI =
    brion::URI( "file://" + TEST_MORPHOLOGY_FILENAME );


// Ellipsis promotes enums to ints, so we need to use int.
#pragma clang diagnostic ignored "-Wnon-pod-varargs"
const int UNDEFINED = brion::SECTION_UNDEFINED;
const int SOMA = brion::SECTION_SOMA;
const int AXON = brion::SECTION_AXON;
const int DENDRITE = brion::SECTION_DENDRITE;
const int APICAL_DENDRITE = brion::SECTION_APICAL_DENDRITE;
}

BOOST_AUTO_TEST_CASE( invalid_open )
{
    BOOST_CHECK_THROW( brion::Morphology( "/bla" ), std::runtime_error );
    BOOST_CHECK_THROW( brion::Morphology( "bla" ), std::runtime_error );

    boost::filesystem::path path( BBP_TESTDATA );
    path /= "local/README";
    BOOST_CHECK_THROW( brion::Morphology( path.string( )),
                       std::runtime_error );
}

BOOST_AUTO_TEST_CASE( h5_invalid_open )
{
    boost::filesystem::path path( BBP_TESTDATA );
    path /= "local/simulations/may17_2011/Control/voltage.h5";
    BOOST_CHECK_THROW( brion::Morphology( path.string( )),
                       std::runtime_error );
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
    BOOST_CHECK_NO_THROW(
        brion::Morphology( file, brion::MORPHOLOGY_VERSION_H5_2, false ));
    BOOST_CHECK_THROW(
        brion::Morphology( file, brion::MORPHOLOGY_VERSION_H5_2, false ),
        std::runtime_error );
    BOOST_CHECK_NO_THROW(
        brion::Morphology( file, brion::MORPHOLOGY_VERSION_H5_2, true ));
    boost::filesystem::remove( file );

    BOOST_CHECK_NO_THROW(
        brion::Morphology( file, brion::MORPHOLOGY_VERSION_H5_2, true ));
    boost::filesystem::remove( file );
}

BOOST_AUTO_TEST_CASE( h5_read_v1 )
{
    boost::filesystem::path path( BBP_TESTDATA );
    path /= "local/morphologies/01.07.08/h5/R-C010306G.h5";

    const brion::Morphology morphology( path.string( ));

    const brion::Vector4fsPtr points =
                                 morphology.readPoints( brion::MORPHOLOGY_RAW );
    BOOST_CHECK_EQUAL( points->size(), 3272 );
    BOOST_CHECK_CLOSE( (*points)[0].x(), -9.0625f, .000001f );
    BOOST_CHECK_CLOSE( (*points)[0].y(), -4.97781f, .0001f );
    BOOST_CHECK_CLOSE( (*points)[0].z(), 0.f, .000001f );
    BOOST_CHECK_CLOSE( (*points)[0].w(), 0.37f, .000001f );

    const brion::Vector2isPtr sections =
                               morphology.readSections( brion::MORPHOLOGY_RAW );
    BOOST_CHECK_EQUAL( sections->size(), 138 );
    BOOST_CHECK_EQUAL( (*sections)[0].x(),  0 );
    BOOST_CHECK_EQUAL( (*sections)[0].y(), -1 );
    BOOST_CHECK_EQUAL( (*sections)[5].x(), 85 );
    BOOST_CHECK_EQUAL( (*sections)[5].y(),  4 );

    const brion::SectionTypesPtr types = morphology.readSectionTypes();
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
        brion::Morphology a( "testv2.h5",
                             brion::MORPHOLOGY_VERSION_UNDEFINED, true );
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
void checkEqualArrays( const std::vector< T >& array, const size_t length, ... )
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

template< typename T >
void _checkCloseArrays( const std::vector< T >& array,
                        const size_t length, va_list args )
{
    for( size_t i = 0; i != length; ++i )
    {
        const T& v = ( T )va_arg( args, typename VaArgsType< T >::type );
        std::ostringstream os;
        os << array[i] << " != " << v << " at " << i;
        BOOST_CHECK_MESSAGE( array[i].equals( v ), os.str( ));
    }
}

template< typename T >
void checkCloseArrays( const std::vector< T >& array,
                       const size_t length, ... )
{
    BOOST_CHECK_EQUAL( array.size(), length );
    va_list args;
    va_start( args, length );
    _checkCloseArrays( array, length, args );
    va_end( args );
}

template< typename T >
void checkCloseArrays( const std::vector< T >& array1,
                       const std::vector< T >& array2 )
{
    BOOST_CHECK_EQUAL( array1.size(), array2.size() );
    for( size_t i = 0; i != std::min( array1.size(), array2.size( )); ++i )
        BOOST_CHECK_CLOSE( array1[i], array2[i], 2e-5f);
}

template< typename T, long unsigned int M >
void checkCloseArrays( const std::vector< vmml::vector< M, T > >& array1,
                       const std::vector< vmml::vector< M, T > >& array2 )
{
    BOOST_CHECK_EQUAL( array1.size(), array2.size() );
    for( size_t i = 0; i != std::min( array1.size(), array2.size( )); ++i )
        BOOST_CHECK_SMALL(( array1[i] - array2[i] ).length( ), 0.00001f);
}

template< typename T >
void checkCloseArraysUptoN( const std::vector< T >& array,
                            const size_t length, ... )
{
    BOOST_CHECK( array.size() >= length );
    va_list args;
    va_start( args, length );
    _checkCloseArrays( array, length, args );
    va_end( args );
}

brion::uint32_ts getSectionIDs( const brain::neuron::Sections& sections )
{
    brion::uint32_ts result;
    BOOST_FOREACH( const brain::neuron::Section& section, sections )
        result.push_back( section.getID( ));
    return result;
}

BOOST_AUTO_TEST_CASE( swc_soma )
{
    boost::filesystem::path path( BRION_TESTDATA );
    const brion::MorphologyRepairStage stage = brion::MORPHOLOGY_REPAIRED;
    path /= "swc/soma.swc";

    const brion::Morphology source( path.string( ));
    checkEqualArrays( *source.readPoints( stage ), 1, V4f( 0, 0, 0, 20 ));
    checkEqualArrays( *source.readSections( stage ), 1, V2i( 0, -1 ));
    checkEqualArrays( *source.readSectionTypes(), 1, SOMA );
}

BOOST_AUTO_TEST_CASE( swc_soma_ring )
{
    boost::filesystem::path path( BRION_TESTDATA );
    const brion::MorphologyRepairStage stage = brion::MORPHOLOGY_REPAIRED;
    path /= "swc/soma_ring.swc";

    const brion::Morphology source( path.string( ));
    checkEqualArrays( *source.readPoints( stage ), 5, V4f( 0, 0, 0, 20 ),
                      V4f( 0, 0, 1, 20 ), V4f( 0, 1, 0, 20 ), V4f( 0, 1, 1, 20 ),
                      V4f( 1, 0, 0, 20 ));
    checkEqualArrays( *source.readSections( stage ), 1, V2i( 0, -1 ));
    checkEqualArrays( *source.readSectionTypes(), 1, SOMA );
}

BOOST_AUTO_TEST_CASE( swc_no_soma )
{
    boost::filesystem::path path( BRION_TESTDATA );
    path /= "swc/no_soma.swc";

    BOOST_CHECK_THROW( brion::Morphology( path.string( )),
                       std::runtime_error );
}

BOOST_AUTO_TEST_CASE( swc_two_somas )
{
    boost::filesystem::path path( BRION_TESTDATA );
    path /= "swc/two_somas.swc";

    BOOST_CHECK_THROW( brion::Morphology( path.string( )),
                       std::runtime_error );
}

BOOST_AUTO_TEST_CASE( swc_single_section )
{
    boost::filesystem::path path( BRION_TESTDATA );
    const brion::MorphologyRepairStage stage = brion::MORPHOLOGY_REPAIRED;
    path /= "swc/single_section.swc";

    const brion::Morphology source( path.string( ));

    checkEqualArrays( *source.readPoints( stage ), 5, V4f( 0, 0, 0, 20 ),
                      V4f( 0, 0, 1, 4 ), V4f( 0, 0, 2, 4 ), V4f( 0, 0, 3, 4 ),
                      V4f( 0, 0, 4, 4 ));
    checkEqualArrays( *source.readSections( stage ),
                      2, V2i( 0, -1 ), V2i( 1, 0 ));
    checkEqualArrays( *source.readSectionTypes(), 2, SOMA, AXON );
}

BOOST_AUTO_TEST_CASE( swc_single_section_unordered )
{
    boost::filesystem::path path( BRION_TESTDATA );
    const brion::MorphologyRepairStage stage = brion::MORPHOLOGY_REPAIRED;
    path /= "swc/single_section_unordered.swc";

    const brion::Morphology source( path.string( ));

    checkEqualArrays( *source.readPoints( stage ), 5, V4f( 0, 0, 0, 20 ),
                      V4f( 0, 0, 1, 4 ), V4f( 0, 0, 2, 4 ), V4f( 0, 0, 3, 4 ),
                      V4f( 0, 0, 4, 4 ));
    checkEqualArrays( *source.readSections( stage ),
                      2, V2i( 0, -1 ), V2i( 1, 0 ));
    checkEqualArrays( *source.readSectionTypes(), 2, SOMA, AXON );
}

BOOST_AUTO_TEST_CASE( swc_single_section_missing_segment )
{
    boost::filesystem::path path( BRION_TESTDATA );
    path /= "swc/single_section_missing_segment.swc";

    BOOST_CHECK_THROW( brion::Morphology( path.string( )),
                       std::runtime_error );
}

BOOST_AUTO_TEST_CASE( swc_section_type_changes )
{
    boost::filesystem::path path( BRION_TESTDATA );
    const brion::MorphologyRepairStage stage = brion::MORPHOLOGY_REPAIRED;
    path /= "swc/section_type_changes.swc";

    const brion::Morphology source( path.string( ));

    checkEqualArrays( *source.readPoints( stage ), 7, V4f( 0, 0, 0, 20 ),
                      V4f( 0, 0, 1, 4 ), V4f( 0, 0, 2, 4 ), V4f( 0, 0, 2, 4 ),
                      V4f( 0, 0, 3, 4 ), V4f( 0, 0, 3, 4 ), V4f( 0, 0, 4, 4 ));
    checkEqualArrays( *source.readSections( stage ), 4,
                      V2i( 0, -1 ), V2i( 1, 0 ), V2i( 3, 1 ), V2i( 5, 2 ));
    checkEqualArrays( *source.readSectionTypes(), 4,
                      SOMA, AXON, DENDRITE, APICAL_DENDRITE );
}

BOOST_AUTO_TEST_CASE( swc_first_order_sections )
{
    boost::filesystem::path path( BRION_TESTDATA );
    const brion::MorphologyRepairStage stage = brion::MORPHOLOGY_REPAIRED;
    path /= "swc/first_order_sections.swc";

    const brion::Morphology source( path.string( ));

    checkEqualArrays( *source.readSections( stage ), 4,
                 V2i( 0, -1 ), V2i( 1, 0 ), V2i( 2, 0 ), V2i( 3, 0 ));
    // The tree construction algorithm reserves the order of two sections
    // compared to how they appear in the file
    checkEqualArrays( *source.readPoints( stage ), 4, V4f( 0, 0, 0, 20 ),
                      V4f( 0, 0, 1, 4 ), V4f( 0, 0, 3, 4 ), V4f( 0, 0, 2, 4 ));
    checkEqualArrays( *source.readSectionTypes(), 4,
                      SOMA, AXON, APICAL_DENDRITE, DENDRITE );

}

BOOST_AUTO_TEST_CASE( swc_bifurcation )
{
    boost::filesystem::path path( BRION_TESTDATA );
    const brion::MorphologyRepairStage stage = brion::MORPHOLOGY_REPAIRED;
    path /= "swc/bifurcations.swc";

    const brion::Morphology source( path.string( ));

    checkEqualArrays( *source.readPoints( stage ), 9, V4f( 0, 0, 0, 20 ),
                      V4f( 0, 0, 2, 4 ), V4f( 0, 0, 3, 4 ), V4f( 0, 0, 3, 4 ),
                      V4f( 0, 0, 4, 4 ), V4f( 0, 0, 5, 4 ), V4f( 0, 0, 3, 4 ),
                      V4f( 0, 0, 6, 4 ), V4f( 0, 0, 7, 4 ));
    checkEqualArrays( *source.readSections( stage ), 4,
                      V2i( 0, -1 ), V2i( 1, 0 ), V2i( 3, 1 ), V2i( 6, 1 ));
    checkEqualArrays( *source.readSectionTypes(), 4,
                      SOMA, DENDRITE, APICAL_DENDRITE, APICAL_DENDRITE );
}

BOOST_AUTO_TEST_CASE( swc_end_points )
{
    boost::filesystem::path path( BRION_TESTDATA );
    const brion::MorphologyRepairStage stage = brion::MORPHOLOGY_REPAIRED;
    path /= "swc/end_points.swc";

    const brion::Morphology source( path.string( ));

    checkEqualArrays( *source.readSections( stage ), 6,
                      V2i( 0, -1 ), V2i( 1, 0 ), V2i( 2, 1 ), V2i( 4, 1 ),
                      V2i( 7, 0 ), V2i( 8, 0 ));

    checkEqualArrays( *source.readSectionTypes(), 6,
                      SOMA, AXON, AXON, AXON, UNDEFINED, UNDEFINED );
}

BOOST_AUTO_TEST_CASE( swc_fork_points )
{
    boost::filesystem::path path( BRION_TESTDATA );
    const brion::MorphologyRepairStage stage = brion::MORPHOLOGY_REPAIRED;
    path /= "swc/fork_points.swc";

    const brion::Morphology source( path.string( ));

    checkEqualArrays( *source.readSections( stage ), 6,
                      V2i( 0, -1 ), V2i( 1, 0 ), V2i( 2, 1 ), V2i( 4, 1 ),
                      V2i( 7, 0 ), V2i( 8, 0 ));

    checkEqualArrays( *source.readSectionTypes(), 6,
                      SOMA, AXON, AXON, AXON, UNDEFINED, UNDEFINED );
}

BOOST_AUTO_TEST_CASE( swc_neuron )
{
    boost::filesystem::path path( BRION_TESTDATA );
    path /= "swc/Neuron.swc";

    brion::Morphology neuron( path.string( ));
    BOOST_CHECK_EQUAL( neuron.readPoints( brion::MORPHOLOGY_REPAIRED )->size(),
                       927 );
}

namespace
{
void checkEqualMorphologies( const brain::neuron::Morphology& first,
                             const brion::Morphology& second )
{
    BOOST_CHECK( *second.readPoints( brion::MORPHOLOGY_UNDEFINED ) ==
                 first.getPoints( ));
    BOOST_CHECK( *second.readSections( brion::MORPHOLOGY_UNDEFINED ) ==
                 first.getSections( ));
    BOOST_CHECK( *second.readSectionTypes() == first.getSectionTypes( ));
    BOOST_CHECK( *second.readApicals() == first.getApicals( ));
}
}

BOOST_AUTO_TEST_CASE( v2_morphology_constructors )
{
    boost::shared_ptr< brion::Morphology > raw(
        new brion::Morphology( TEST_MORPHOLOGY_FILENAME ));

    brain::neuron::Morphology morphology( TEST_MORPHOLOGY_URI );
    BOOST_CHECK_EQUAL( morphology.getTransformation(),
                       brain::Matrix4f( ));
    checkEqualMorphologies( morphology, *raw );
    checkEqualMorphologies( brain::neuron::Morphology( *raw ), *raw );

    BOOST_CHECK_THROW( brain::neuron::Morphology( brion::URI( "/mars" )),
                       std::runtime_error);
}

BOOST_AUTO_TEST_CASE( get_section_ids )
{
    brain::neuron::Morphology morphology( TEST_MORPHOLOGY_URI );

    brion::SectionTypes types;
    types.push_back( brion::SECTION_SOMA );
    checkEqualArrays( morphology.getSectionIDs( types ), 1, 0 );

    types.push_back( brion::SECTION_DENDRITE );
    checkEqualArrays( morphology.getSectionIDs( types ),
                      7, 0, 4, 5, 6, 7, 8, 9 );
    types.push_back( brion::SECTION_APICAL_DENDRITE );
    checkEqualArrays( morphology.getSectionIDs( types ),
                      10, 0, 4, 5, 6, 7, 8, 9, 10, 11, 12 );
    types.clear();
    types.push_back( brion::SECTION_AXON );
    types.push_back( brion::SECTION_DENDRITE );
    checkEqualArrays( morphology.getSectionIDs( types ),
                      9, 1, 2, 3, 4, 5, 6, 7, 8, 9 );
}

BOOST_AUTO_TEST_CASE( get_sections )
{
    brain::neuron::Morphology morphology( TEST_MORPHOLOGY_URI );

    BOOST_CHECK_THROW( morphology.getSection( 0 ), std::runtime_error );

    for( size_t i = 1; i < 13; ++i )
        BOOST_CHECK_EQUAL( morphology.getSection( i ).getID(), i );

    brain::neuron::Section section = morphology.getSection( 1 );
    BOOST_CHECK( section == morphology.getSection( 1 ));
    section = morphology.getSection( 2 );
    BOOST_CHECK( section != morphology.getSection( 1 ));
    BOOST_CHECK( section == morphology.getSection( 2 ));

    for( size_t i = 1; i < 4; ++i )
        BOOST_CHECK_EQUAL( morphology.getSection( i ).getType(),
                           brion::SECTION_AXON );
    for( size_t i = 4; i < 10; ++i )
        BOOST_CHECK_EQUAL( morphology.getSection( i ).getType(),
                           brion::SECTION_DENDRITE );
    for( size_t i = 10; i < 13; ++i )
        BOOST_CHECK_EQUAL( morphology.getSection( i ).getType(),
                           brion::SECTION_APICAL_DENDRITE );
}

BOOST_AUTO_TEST_CASE( get_section_samples )
{
    brain::neuron::Morphology morphology( TEST_MORPHOLOGY_URI );

    brion::Vector4fs points;
    for( size_t i = 0; i != 11; ++i)
    {
        float i2 = i * i;
        points.push_back(
            brion::Vector4f(0, -i2 / 20.0, i2 / 20.0, 0.5 + i2 /1000.0));
    }
    checkCloseArrays( morphology.getSection( 1 ).getSamples(), points );

    points.clear();
    for( size_t i = 0; i != 11; ++i)
    {
        float i2 = i * i;
        points.push_back(
            brion::Vector4f(i2 / 20.0, 0, i2 / 20.0, 0.5 + i2 /1000.0));
    }
    checkCloseArrays( morphology.getSection( 4 ).getSamples(), points );

    points.clear();
    for( size_t i = 0; i != 11; ++i)
    {
        float i2 = i * i;
        points.push_back(
            brion::Vector4f(-i2 / 20.0, 0, i2 / 20.0, 0.5 + i2 /1000.0));
    }
    checkCloseArrays( morphology.getSection( 7 ).getSamples(), points );

    points.clear();
    for( size_t i = 0; i != 11; ++i)
    {
        float i2 = i * i;
        points.push_back(
            brion::Vector4f(0, i2 / 20.0, i2 / 20.0, 0.5 + i2 /1000.0));
    }
    checkCloseArrays( morphology.getSection( 10 ).getSamples(), points );
}

BOOST_AUTO_TEST_CASE( get_section_distances_to_soma )
{
    brain::neuron::Morphology morphology( TEST_MORPHOLOGY_URI );

    uint32_t sections[] = {1, 4, 7, 10};

    for( size_t i = 0; i != 4; ++i)
    {
        uint32_t section = sections[i];
        BOOST_CHECK_EQUAL(
            morphology.getSection( section ).getDistanceToSoma(), 0 );
        const float length = std::sqrt( 5 * 5 * 2 );
        BOOST_CHECK_CLOSE(
            morphology.getSection( section ).getLength(), length, 1e-5 );

        // The distance to the soma of the next section is equal to the length
        // of its parent
        BOOST_CHECK_CLOSE(
            morphology.getSection( section + 1 ).getDistanceToSoma(),
            length, 1e-5 );

        brion::floats reference;
        for( size_t j = 0; j != 11; ++j)
        {
            const float p = j*j / 20.0;
            reference.push_back( std::sqrt( p * p * 2 ));
        }
        checkCloseArrays(
            morphology.getSection( section ).getSampleDistancesToSoma( ),
            reference );
    }
}

BOOST_AUTO_TEST_CASE( get_soma_geomery )
{
    brain::neuron::Morphology morphology( TEST_MORPHOLOGY_URI );

    const brain::neuron::Soma soma = morphology.getSoma();
    checkEqualArrays( soma.getProfilePoints(), 4,
                      V4f( .1, 0, 0, .1 ), V4f( 0, .1, 0, .1 ),
                      V4f( -.1, 0, 0, .1 ), V4f( 0, -.1, 0, .1 ));

    BOOST_CHECK_CLOSE( soma.getMeanRadius(), 0.1, 1e-5 );
    BOOST_CHECK_EQUAL( soma.getCentroid(), V3f::ZERO );

    brain::Matrix4f matrix;
    matrix.setTranslation( V3f( 2, 0, 0 ));
    brain::neuron::Morphology transformed( TEST_MORPHOLOGY_URI, matrix );
    BOOST_CHECK_MESSAGE( transformed.getSoma().getCentroid().equals(V3f( 2,0,0 )),
                         transformed.getSoma().getCentroid( ));

}

BOOST_AUTO_TEST_CASE( get_section_samples_by_positions )
{
    brain::neuron::Morphology morphology( TEST_MORPHOLOGY_URI );

    brion::floats points;
    for( float p = 0.0; p <= 1.0; p += 0.2 )
        points.push_back( p );

    checkCloseArrays( morphology.getSection( 1 ).getSamples( points ), 6,
        V4f( 0, 0, 0, .5 ), V4f( 0, -1, 1, .52 ), V4f( 0, -2, 2, .54 ),
        V4f( 0, -3, 3, .56 ), V4f( 0, -4, 4, .58 ), V4f(  0, -5, 5, .6 ));

    checkCloseArrays( morphology.getSection( 4 ).getSamples( points ), 6,
        V4f( 0, 0, 0, .5 ), V4f( 1, 0, 1, .52 ), V4f( 2, 0, 2, .54 ),
        V4f( 3, 0, 3, .56 ), V4f( 4, 0, 4, .58 ), V4f(  5, 0, 5, .6 ));

    checkCloseArrays( morphology.getSection( 7 ).getSamples( points ), 6,
        V4f( 0, 0, 0, .5 ), V4f( -1, 0, 1, .52 ), V4f( -2, 0, 2, .54 ),
        V4f( -3, 0, 3, .56 ), V4f( -4, 0, 4, .58 ), V4f( -5, 0, 5, .6 ));

    checkCloseArrays( morphology.getSection( 10 ).getSamples( points ), 6,
        V4f( 0, 0, 0, .5 ), V4f( 0, 1, 1, .52 ), V4f( 0, 2, 2, .54 ),
        V4f( 0, 3, 3, .56 ), V4f( 0, 4, 4, .58 ), V4f(  0, 5, 5, .6 ));
}

BOOST_AUTO_TEST_CASE( morphology_hierarchy )
{
    brain::neuron::Morphology morphology( TEST_MORPHOLOGY_URI );

    BOOST_CHECK( !morphology.getSection( 1 ).hasParent( ));
    BOOST_CHECK( !morphology.getSection( 4 ).hasParent( ));
    BOOST_CHECK_EQUAL( morphology.getSection( 2 ).getParent().getID(), 1 );
    BOOST_CHECK_EQUAL( morphology.getSection( 3 ).getParent().getID(), 1 );
    BOOST_CHECK_EQUAL( morphology.getSection( 5 ).getParent().getID(), 4 );
    BOOST_CHECK_EQUAL( morphology.getSection( 6 ).getParent().getID(), 4 );

    checkEqualArrays( getSectionIDs( morphology.getSoma().getChildren( )),
                      4, 1, 4, 7, 10 );
    checkEqualArrays( getSectionIDs( morphology.getSection( 1 ).getChildren( )),
                      2, 2, 3 );
    checkEqualArrays( getSectionIDs( morphology.getSection( 4 ).getChildren( )),
                      2, 5, 6 );
    BOOST_CHECK( morphology.getSection( 5 ).getChildren().empty( ));
}

BOOST_AUTO_TEST_CASE( transform_with_matrix )
{
    brain::Matrix4f matrix;
    matrix.rotate_z( M_PI * 0.5 );
    brain::neuron::Morphology rotated( TEST_MORPHOLOGY_URI, matrix );
    checkCloseArraysUptoN( rotated.getPoints(), 4,
      V4f( .0, .1, .0, .1 ), V4f( -.1, .0, .0, .1 ),
      V4f( .0, -.1, .0, .1 ), V4f( .1, .0, .0, .1 ));

    matrix = brain::Matrix4f();
    matrix.rotate_z( M_PI * 0.5 );
    matrix.setTranslation( V3f( 2, 0, 0 ));
    brain::neuron::Morphology transformed( TEST_MORPHOLOGY_URI, matrix );
    BOOST_CHECK_EQUAL( transformed.getTransformation(), matrix );
    checkCloseArraysUptoN( transformed.getPoints(), 4,
      V4f( 2., .1, .0, .1 ), V4f( 1.9, .0, .0, .1 ),
      V4f( 2., -.1, .0, .1 ), V4f( 2.1, .0, .0, .1 ));
}
