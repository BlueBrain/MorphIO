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

#include <boost/noncopyable.hpp>
#include <brion/api.h>
#include <brion/types.h>

namespace brion
{
namespace detail
{
class Mesh;
}

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
    BRION_API ~Mesh();

    /** @name Read API */
    //@{
    /** Open the given source to a mesh file for reading.
     *
     * @param source filepath to mesh file
     * @throw std::runtime_error if file is not a valid mesh file
     */
    BRION_API explicit Mesh(const std::string& source);

    /** @return the version of the mesh */
    BRION_API MeshVersion getVersion() const;

    /** @return the number of vertices, is never 0 for a valid mesh */
    BRION_API size_t getNumVertices() const;

    /** @return vertices describing the membrane/surface mesh */
    BRION_API Vector3fsPtr readVertices() const;

    /** @return section indices for each vertex */
    BRION_API uint16_tsPtr readVertexSections() const;

    /** @return relative distances in the section for each vertex */
    BRION_API floatsPtr readVertexDistances() const;

    /** @return the number of triangles, can be 0 if a triangle strip is used */
    BRION_API size_t getNumTriangles() const;

    /** @return triangles describing the membrane/surface mesh */
    BRION_API uint32_tsPtr readTriangles() const;

    /** @return section indices for each triangle */
    BRION_API uint16_tsPtr readTriangleSections() const;

    /** @return relative distances in the section for each triangle */
    BRION_API floatsPtr readTriangleDistances() const;

    /** @return the length of the triangle strip, can be 0 if
        triangles are used */
    BRION_API size_t getTriStripLength() const;

    /** @return triangle strip describing the membrane/surface mesh */
    BRION_API uint32_tsPtr readTriStrip() const;

    /** @return the number of normals, is 0 for binary meshes */
    BRION_API size_t getNumNormals() const;

    /** @return per-vertex normals calculated by the mesh generation */
    BRION_API Vector3fsPtr readNormals() const;

    /** Get the number of structural meshes of the given type.
     *
     * Note that binary meshes do not support structural mesh information.
     *
     * @param type the mesh structure type to look for
     * @return number of structural meshes
     */
    BRION_API size_t getNumStructures(MeshStructure type) const;

    /** Get the vertices of the given structure.
     *
     * Note that binary meshes do not support structural mesh information.
     *
     * @param type the mesh structure type to look for
     * @param index the index of the mesh structure
     * @return vertices describing the structural mesh
     */
    BRION_API Vector3fsPtr readStructureVertices(MeshStructure type,
                                                 size_t index) const;

    /**  Get the triangles of the given structure.
     *
     * Note that binary meshes do not support structural mesh information.
     *
     * @param type the mesh structure type to look for
     * @param index the index of the mesh structure
     * @return triangles describing the structural mesh
     */
    BRION_API uint32_tsPtr readStructureTriangles(MeshStructure type,
                                                  size_t index) const;

    /** Get the triangle strip of the given structure.
     *
     * Note that binary meshes do not support structural mesh information.
     *
     * @param type the mesh structure type to look for
     * @param index the index of the mesh structure
     * @return triangle strip describing the structural mesh
     */
    BRION_API uint32_tsPtr readStructureTriStrip(MeshStructure type,
                                                 size_t index) const;
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
    BRION_API Mesh(const std::string& source, MeshFormat format,
                   bool overwrite = false,
                   MeshVersion version = MESH_VERSION_1);

    /** Write vertices describing the surface/membrane mesh of a neuron.
     *
     * @param vertices the vertices describing the surface/membrane mesh
     * @throw std::runtime_error if object not created with write ctor
     */
    BRION_API void writeVertices(const Vector3fs& vertices);

    /** Write section indices for each vertex of the surface/membrane mesh.
     *
     * @param vSections the section indices for each vertex
     * @throw std::runtime_error if number of vertices does not match
     * @throw std::runtime_error if object not created with write ctor
     */
    BRION_API void writeVertexSections(const uint16_ts& vSections);

    /** Write relative distances in the section for each vertex of the
     *  surface/membrane mesh.
     *
     * @param vDistances the relative distances in the section for each vertex
     * @throw std::runtime_error if number of vertices does not match
     * @throw std::runtime_error if object not created with write ctor
     */
    BRION_API void writeVertexDistances(const floats& vDistances);

    /** Write triangles describing the surface/membrane mesh of a neuron.
     *
     * @param triangles the triangles describing the surface/membrane mesh
     * @throw std::runtime_error if vertices were not written before
     * @throw std::runtime_error if object not created with write ctor
     */
    BRION_API void writeTriangles(const uint32_ts& triangles);

    /** Write section indices for each triangle of the surface/membrane mesh.
     *
     * @param tSections the section indices for each triangle
     * @throw std::runtime_error if number of triangles does not match
     * @throw std::runtime_error if called for binary meshes
     * @throw std::runtime_error if object not created with write ctor
     */
    BRION_API void writeTriangleSections(const uint16_ts& tSections);

    /** Write relative distances in the section for each triangle of the
     *  surface/membrane mesh.
     *
     * @param tDistances the relative distances in the section for each triangle
     * @throw std::runtime_error if number of triangles does not match
     * @throw std::runtime_error if called for binary meshes
     * @throw std::runtime_error if object not created with write ctor
     */
    BRION_API void writeTriangleDistances(const floats& tDistances);

    /** Write triangle strip describing the surface/membrane mesh of a neuron.
     *
     * @param tristrip the triangle strip describing the surface/membrane mesh
     * @throw std::runtime_error if vertices were not written before
     * @throw std::runtime_error if object not created with write ctor
     */
    BRION_API void writeTriStrip(const uint32_ts& tristrip);

    /** Write per-vertex normals for the surface/membrane mesh of a neuron.
     *
     * @param normals the per-vertex normals for the surface/membrane mesh
     * @throw std::runtime_error if number of vertices does not match
     * @throw std::runtime_error if called for binary meshes
     * @throw std::runtime_error if object not created with write ctor
     */
    BRION_API void writeNormals(const Vector3fs& normals);

    /** Write vertices for one type of structural mesh.
     *
     * @param vertices the vertices describing the structural mesh
     * @param type the type of the structural mesh
     * @param index the index of the structural mesh
     * @throw std::runtime_error if called for binary meshes
     * @throw std::runtime_error if object not created with write ctor
     */
    BRION_API void writeStructureVertices(const Vector3fs& vertices,
                                          MeshStructure type, size_t index);

    /** Write triangles for one type of structural mesh.
     *
     * @param triangles the triangles describing the structural mesh
     * @param type the type of the structural mesh
     * @param index the index of the structural mesh
     * @throw std::runtime_error if called for binary meshes
     * @throw std::runtime_error if object not created with write ctor
     */
    BRION_API void writeStructureTriangles(const uint32_ts& triangles,
                                           MeshStructure type, size_t index);

    /** Write the triangle strip for one type of structural mesh.
     *
     * @param tristrip the triangle strip describing the structural mesh
     * @param type the type of the structural mesh
     * @param index the index of the structural mesh
     * @throw std::runtime_error if called for binary meshes
     * @throw std::runtime_error if object not created with write ctor
     */
    BRION_API void writeStructureTriStrip(const uint32_ts& tristrip,
                                          MeshStructure type, size_t index);

    /** Flush data to output. */
    BRION_API void flush();
    //@}

private:
    detail::Mesh* _impl;
};
}

#endif
