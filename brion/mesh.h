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
 */

#ifndef BRION_MESH
#define BRION_MESH

#include <brion/types.h>
#include <boost/noncopyable.hpp>

namespace brion
{

namespace detail { class Mesh; }

/** Read & write access to a Mesh file.
 *
 * The supported types are binary and HDF5 files.
 *
 * Following RAII, this class is ready to use after the creation and will ensure
 * release of resources upon destruction.
 */
class Mesh : public boost::noncopyable
{
public:
    /** Close mesh file. */
    ~Mesh();

    /** @name Read API */
    //@{
    /** Open the given source to a mesh file for reading.
     *
     * @param source filepath to mesh file
     * @throw std::runtime_error if file is not a valid mesh file
     */
    explicit Mesh( const std::string& source );

    /** @return the version of the mesh */
    MeshVersion getVersion() const;

    /** @return the number of vertices, is never 0 for a valid mesh */
    size_t getNumVertices() const;

    /** @return vertices describing the membrane/surface mesh */
    Vector3fsPtr readVertices() const;

    /** @return section indices for each vertex */
    uint16_tsPtr readVertexSections() const;

    /** @return relative distances in the section for each vertex */
    floatsPtr readVertexDistances() const;

    /** @return the number of triangles, can be 0 if a triangle strip is used */
    size_t getNumTriangles() const;

    /** @return triangles describing the membrane/surface mesh */
    uint32_tsPtr readTriangles() const;

    /** @return section indices for each triangle */
    uint16_tsPtr readTriangleSections() const;

    /** @return relative distances in the section for each triangle */
    floatsPtr readTriangleDistances() const;

    /** @return the length of the triangle strip, can be 0 if
        triangles are used */
    size_t getTriStripLength() const;

    /** @return triangle strip describing the membrane/surface mesh */
    uint32_tsPtr readTriStrip() const;

    /** @return the number of normals, is 0 for binary meshes */
    size_t getNumNormals() const;

    /** @return per-vertex normals calculated by the mesh generation */
    Vector3fsPtr readNormals() const;

    /** Get the number of structural meshes of the given type.
     *
     * Note that binary meshes do not support structural mesh information.
     *
     * @param type the mesh structure type to look for
     * @return number of structural meshes
     */
    size_t getNumStructures( const MeshStructure type ) const;

    /** Get the vertices of the given structure.
     *
     * Note that binary meshes do not support structural mesh information.
     *
     * @param type the mesh structure type to look for
     * @param index the index of the mesh structure
     * @return vertices describing the structural mesh
     */
    Vector3fsPtr readStructureVertices( const MeshStructure type,
                                        const size_t index ) const;

    /**  Get the triangles of the given structure.
     *
     * Note that binary meshes do not support structural mesh information.
     *
     * @param type the mesh structure type to look for
     * @param index the index of the mesh structure
     * @return triangles describing the structural mesh
     */
    uint32_tsPtr readStructureTriangles( const MeshStructure type,
                                         const size_t index ) const;

    /** Get the triangle strip of the given structure.
     *
     * Note that binary meshes do not support structural mesh information.
     *
     * @param type the mesh structure type to look for
     * @param index the index of the mesh structure
     * @return triangle strip describing the structural mesh
     */
    uint32_tsPtr readStructureTriStrip( const MeshStructure type,
                                        const size_t index ) const;
    //@}


    /** @name Write API */
    //@{
    /** Open the given mesh file for write access.
     *
     * @param source filepath to mesh file
     * @param format output format of the mesh
     * @param overwrite true to allow overwrite of existing file
     * @param version the output file format version
     * @throw std::runtime_error if file could not be opened for write access
     */
    Mesh( const std::string& source, const MeshFormat format,
          const bool overwrite = false,
          const MeshVersion version = MESH_VERSION_1 );

    /** Write vertices describing the surface/membrane mesh of a neuron.
     *
     * @param vertices the vertices describing the surface/membrane mesh
     * @throw std::runtime_error if object not created with write ctor
     */
    void writeVertices( const Vector3fs& vertices );

    /** Write section indices for each vertex of the surface/membrane mesh.
     *
     * @param vSections the section indices for each vertex
     * @throw std::runtime_error if number of vertices does not match
     * @throw std::runtime_error if object not created with write ctor
     */
    void writeVertexSections( const uint16_ts& vSections );

    /** Write relative distances in the section for each vertex of the
     *  surface/membrane mesh.
     *
     * @param vDistances the relative distances in the section for each vertex
     * @throw std::runtime_error if number of vertices does not match
     * @throw std::runtime_error if object not created with write ctor
     */
    void writeVertexDistances( const floats& vDistances );

    /** Write triangles describing the surface/membrane mesh of a neuron.
     *
     * @param triangles the triangles describing the surface/membrane mesh
     * @throw std::runtime_error if vertices were not written before
     * @throw std::runtime_error if object not created with write ctor
     */
    void writeTriangles( const uint32_ts& triangles );

    /** Write section indices for each triangle of the surface/membrane mesh.
     *
     * @param tSections the section indices for each triangle
     * @throw std::runtime_error if number of triangles does not match
     * @throw std::runtime_error if called for binary meshes
     * @throw std::runtime_error if object not created with write ctor
     */
    void writeTriangleSections( const uint16_ts& tSections );

    /** Write relative distances in the section for each triangle of the
     *  surface/membrane mesh.
     *
     * @param tDistances the relative distances in the section for each triangle
     * @throw std::runtime_error if number of triangles does not match
     * @throw std::runtime_error if called for binary meshes
     * @throw std::runtime_error if object not created with write ctor
     */
    void writeTriangleDistances( const floats& tDistances );

    /** Write triangle strip describing the surface/membrane mesh of a neuron.
     *
     * @param tristrip the triangle strip describing the surface/membrane mesh
     * @throw std::runtime_error if vertices were not written before
     * @throw std::runtime_error if object not created with write ctor
     */
    void writeTriStrip( const uint32_ts& tristrip );

    /** Write per-vertex normals for the surface/membrane mesh of a neuron.
     *
     * @param normals the per-vertex normals for the surface/membrane mesh
     * @throw std::runtime_error if number of vertices does not match
     * @throw std::runtime_error if called for binary meshes
     * @throw std::runtime_error if object not created with write ctor
     */
    void writeNormals( const Vector3fs& normals );

    /** Write vertices for one type of structural mesh.
     *
     * @param vertices the vertices describing the structural mesh
     * @param type the type of the structural mesh
     * @param index the index of the structural mesh
     * @throw std::runtime_error if called for binary meshes
     * @throw std::runtime_error if object not created with write ctor
     */
    void writeStructureVertices( const Vector3fs& vertices,
                                 const MeshStructure type, const size_t index );

    /** Write triangles for one type of structural mesh.
     *
     * @param triangles the triangles describing the structural mesh
     * @param type the type of the structural mesh
     * @param index the index of the structural mesh
     * @throw std::runtime_error if called for binary meshes
     * @throw std::runtime_error if object not created with write ctor
     */
    void writeStructureTriangles( const uint32_ts& triangles,
                                  const MeshStructure type, const size_t index);

    /** Write the triangle strip for one type of structural mesh.
     *
     * @param tristrip the triangle strip describing the structural mesh
     * @param type the type of the structural mesh
     * @param index the index of the structural mesh
     * @throw std::runtime_error if called for binary meshes
     * @throw std::runtime_error if object not created with write ctor
     */
    void writeStructureTriStrip( const uint32_ts& tristrip,
                                 const MeshStructure type, const size_t index);

    /** Flush data to output. */
    void flush();
    //@}

private:
    detail::Mesh* _impl;
};

}

#endif
