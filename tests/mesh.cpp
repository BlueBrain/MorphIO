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

#define BOOST_TEST_MODULE Mesh
#include <boost/filesystem/path.hpp>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE( test_invalid_open )
{
    BOOST_CHECK_THROW( brion::Mesh( "/bla" ), std::runtime_error );
    BOOST_CHECK_THROW( brion::Mesh( "bla" ), std::runtime_error );

    boost::filesystem::path path( BBP_TESTDATA );
    path /= "local/README";
    BOOST_CHECK_THROW( brion::Mesh( path.string( )), std::runtime_error );

    path = BBP_TESTDATA;
    path /= "local/simulations/may17_2011/Control/voltage.h5";
    BOOST_CHECK_THROW( brion::Mesh( path.string( )), std::runtime_error );

    BOOST_CHECK_THROW( brion::Mesh( path.string(), brion::MESHFORMAT_BINARY ),
                       std::runtime_error );
    BOOST_CHECK_THROW( brion::Mesh( path.string(), brion::MESHFORMAT_HDF5 ),
                       std::runtime_error );
}

BOOST_AUTO_TEST_CASE( test_illegal_write )
{
    boost::filesystem::path path( BBP_TESTDATA );
    path /= "local/meshes/08.05.09/high/TXT/R-C010306G.bin";
    brion::Mesh mesh( path.string( ));

    BOOST_CHECK_THROW( mesh.writeVertices( brion::Vector3fs( )),
                       std::runtime_error );
    BOOST_CHECK_THROW( mesh.writeVertexSections( brion::uint16_ts( )),
                       std::runtime_error );
    BOOST_CHECK_THROW( mesh.writeVertexDistances( brion::floats( )),
                       std::runtime_error );
    BOOST_CHECK_THROW( mesh.writeTriangles( brion::uint32_ts( )),
                       std::runtime_error );
    BOOST_CHECK_THROW( mesh.writeTriangleSections( brion::uint16_ts( )),
                       std::runtime_error );
    BOOST_CHECK_THROW( mesh.writeTriangleDistances( brion::floats( )),
                       std::runtime_error );
    BOOST_CHECK_THROW( mesh.writeTriStrip( brion::uint32_ts( )),
                       std::runtime_error );
    BOOST_CHECK_THROW( mesh.writeNormals( brion::Vector3fs( )),
                       std::runtime_error );
    BOOST_CHECK_THROW( mesh.writeStructureVertices( brion::Vector3fs(),
                                                    brion::MESH_SPINES, 0 ),
                       std::runtime_error );
    BOOST_CHECK_THROW( mesh.writeStructureTriangles( brion::uint32_ts(),
                                                     brion::MESH_SPINES, 0 ),
                       std::runtime_error );
    BOOST_CHECK_THROW( mesh.writeStructureTriStrip( brion::uint32_ts(),
                                                     brion::MESH_SPINES, 0 ),
                       std::runtime_error );
    BOOST_CHECK_THROW( mesh.flush(), std::runtime_error );
}

BOOST_AUTO_TEST_CASE( test_invalid_write )
{
    boost::filesystem::path path( BBP_TESTDATA );
    path /= "local/meshes/08.05.09/high/TXT/R-C010306G.bin";
    brion::Mesh mesh( path.string( ));

    brion::Mesh out( "testmesh.bin", brion::MESHFORMAT_BINARY, true );
    BOOST_CHECK_THROW( out.writeVertexSections( *mesh.readVertexSections( )),
                       std::runtime_error );
    BOOST_CHECK_THROW( out.writeVertexDistances( *mesh.readVertexDistances( )),
                       std::runtime_error );
    BOOST_CHECK_THROW( out.writeTriangles( *mesh.readTriangles( )),
                       std::runtime_error );
    BOOST_CHECK_THROW( out.writeTriStrip( *mesh.readTriStrip( )),
                       std::runtime_error );
}

BOOST_AUTO_TEST_CASE( test_read_binary )
{
    boost::filesystem::path path( BBP_TESTDATA );
    path /= "local/meshes/08.05.09/high/TXT/R-C010306G.bin";
    brion::Mesh mesh( path.string( ));

    BOOST_CHECK_EQUAL( mesh.getVersion(), brion::MESH_VERSION_1 );
    BOOST_CHECK_EQUAL( mesh.getNumVertices(), 38618 );
    BOOST_CHECK_EQUAL( mesh.getNumTriangles(), 77232 );
    BOOST_CHECK_EQUAL( mesh.getTriStripLength(), 111380 );
    BOOST_CHECK_EQUAL( mesh.getNumNormals(), 0 );

    brion::Vector3fsPtr vertices = mesh.readVertices();
    BOOST_CHECK_EQUAL( vertices->size(), mesh.getNumVertices( ));

    brion::uint16_tsPtr vSections = mesh.readVertexSections();
    BOOST_CHECK_EQUAL( vSections->size(), mesh.getNumVertices( ));

    brion::floatsPtr vDistances = mesh.readVertexDistances();
    BOOST_CHECK_EQUAL( vDistances->size(), mesh.getNumVertices( ));

    brion::uint32_tsPtr triangles = mesh.readTriangles();
    BOOST_CHECK_EQUAL( triangles->size(), mesh.getNumTriangles() * 3 );

    brion::uint16_tsPtr tSections = mesh.readTriangleSections();
    BOOST_CHECK_EQUAL( tSections->size(), 0 );

    brion::floatsPtr tDistances = mesh.readTriangleDistances();
    BOOST_CHECK_EQUAL( tDistances->size(), 0 );

    brion::uint32_tsPtr tristrip = mesh.readTriStrip();
    BOOST_CHECK_EQUAL( tristrip->size(), mesh.getTriStripLength() );

    brion::Vector3fsPtr normals = mesh.readNormals();
    BOOST_CHECK_EQUAL( normals->size(), mesh.getNumNormals( ));

    for( size_t i = 0; i < brion::MESH_ALL; ++i )
    {
        BOOST_CHECK_EQUAL( mesh.getNumStructures( brion::MeshStructure( i )),
                           0 );
    }
}

BOOST_AUTO_TEST_CASE( test_write_binary )
{
    boost::filesystem::path path( BBP_TESTDATA );
    path /= "local/meshes/08.05.09/high/TXT/R-C010306G.bin";
    brion::Mesh mesh( path.string( ));

    brion::Vector3fsPtr vertices = mesh.readVertices();
    brion::uint16_tsPtr vSections = mesh.readVertexSections();
    brion::floatsPtr vDistances = mesh.readVertexDistances();
    brion::uint32_tsPtr triangles = mesh.readTriangles();
    brion::uint32_tsPtr tristrip = mesh.readTriStrip();

    {
        brion::Mesh a( "testmesh.bin", brion::MESHFORMAT_BINARY, true );
        a.writeVertices( *vertices );
        a.writeVertexSections( *vSections );
        a.writeVertexDistances( *vDistances );
        a.writeTriangles( *triangles );
        a.writeTriStrip( *tristrip );
    }

    const brion::Mesh source2( "testmesh.bin" );
    const brion::MeshVersion version = source2.getVersion();
    brion::Vector3fsPtr vertices2 = source2.readVertices();
    brion::uint16_tsPtr vSections2 = source2.readVertexSections();
    brion::floatsPtr vDistances2 = source2.readVertexDistances();
    brion::uint32_tsPtr triangles2 = source2.readTriangles();
    brion::uint32_tsPtr tristrip2 = source2.readTriStrip();

    BOOST_CHECK( version == brion::MESH_VERSION_1 );
    BOOST_CHECK( *vertices == *vertices2 );
    BOOST_CHECK( *vSections == *vSections2 );
    BOOST_CHECK( *vDistances == *vDistances2 );
    BOOST_CHECK( *triangles == *triangles2 );
    BOOST_CHECK( *tristrip == *tristrip2 );
}

BOOST_AUTO_TEST_CASE( test_read_binary_v2 )
{
    boost::filesystem::path path( BBP_TESTDATA );
    path /= "local/meshes/08.05.09/high/TXT/R-C010306G-v2.bin";
    const brion::Mesh mesh( path.string( ));

    BOOST_CHECK_EQUAL( mesh.getVersion(), brion::MESH_VERSION_2 );
    BOOST_CHECK_EQUAL( mesh.getNumVertices(), 105394 );
    BOOST_CHECK_EQUAL( mesh.getNumTriangles(), 210784 );
    BOOST_CHECK_EQUAL( mesh.getTriStripLength(), 417105 );
    BOOST_CHECK_EQUAL( mesh.getNumNormals(), 0 );

    const brion::Vector3fsPtr vertices = mesh.readVertices();
    BOOST_CHECK_EQUAL( vertices->size(), mesh.getNumVertices( ));

    const brion::uint16_tsPtr vSections = mesh.readVertexSections();
    BOOST_CHECK_EQUAL( vSections->size(), mesh.getNumVertices( ));

    const brion::floatsPtr vDistances = mesh.readVertexDistances();
    BOOST_CHECK_EQUAL( vDistances->size(), mesh.getNumVertices( ));

    const brion::uint32_tsPtr triangles = mesh.readTriangles();
    BOOST_CHECK_EQUAL( triangles->size(), mesh.getNumTriangles() * 3 );

    const brion::uint16_tsPtr tSections = mesh.readTriangleSections();
    BOOST_CHECK_EQUAL( tSections->size(), 0 );

    const brion::floatsPtr tDistances = mesh.readTriangleDistances();
    BOOST_CHECK_EQUAL( tDistances->size(), 0 );

    const brion::uint32_tsPtr tristrip = mesh.readTriStrip();
    BOOST_CHECK_EQUAL( tristrip->size(), mesh.getTriStripLength() );

    const brion::Vector3fsPtr normals = mesh.readNormals();
    BOOST_CHECK_EQUAL( normals->size(), mesh.getNumNormals( ));

    for( size_t i = 0; i < brion::MESH_ALL; ++i )
    {
        BOOST_CHECK_EQUAL( mesh.getNumStructures( brion::MeshStructure( i )),
                           0 );
    }
}

BOOST_AUTO_TEST_CASE( test_write_binary_v2 )
{
    boost::filesystem::path path( BBP_TESTDATA );
    path /= "local/meshes/08.05.09/high/TXT/R-C010306G-v2.bin";
    brion::Mesh mesh( path.string( ));

    const brion::Vector3fsPtr vertices = mesh.readVertices();
    const brion::uint16_tsPtr vSections = mesh.readVertexSections();
    const brion::floatsPtr vDistances = mesh.readVertexDistances();
    const brion::uint32_tsPtr triangles = mesh.readTriangles();
    const brion::uint32_tsPtr tristrip = mesh.readTriStrip();

    {
        brion::Mesh a( "testmesh.bin", brion::MESHFORMAT_BINARY, true,
                       brion::MESH_VERSION_2 );
        a.writeVertices( *vertices );
        a.writeVertexSections( *vSections );
        a.writeVertexDistances( *vDistances );
        a.writeTriangles( *triangles );
        a.writeTriStrip( *tristrip );
    }

    const brion::Mesh source2( "testmesh.bin" );
    const brion::MeshVersion version = source2.getVersion();
    const brion::Vector3fsPtr vertices2 = source2.readVertices();
    const brion::uint16_tsPtr vSections2 = source2.readVertexSections();
    const brion::floatsPtr vDistances2 = source2.readVertexDistances();
    const brion::uint32_tsPtr triangles2 = source2.readTriangles();
    const brion::uint32_tsPtr tristrip2 = source2.readTriStrip();

    BOOST_CHECK( version == brion::MESH_VERSION_2 );
    BOOST_CHECK( *vertices == *vertices2 );
    BOOST_CHECK( *vSections == *vSections2 );
    BOOST_CHECK( *vDistances == *vDistances2 );
    BOOST_CHECK( *triangles == *triangles2 );
    BOOST_CHECK( *tristrip == *tristrip2 );
}
