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

#ifndef BRION_DETAIL_MESH
#define BRION_DETAIL_MESH

namespace brion
{
namespace detail
{
class Mesh
{
public:
    const std::string _source;
    const bool _write;
    MeshVersion _version;

    explicit Mesh(const std::string& source)
        : _source(source)
        , _write(false)
        , _version(MESH_VERSION_1)
    {
    }
    Mesh(const std::string& source, const MeshVersion version)
        : _source(source)
        , _write(true)
        , _version(version)
    {
    }
    virtual ~Mesh() {}
    MeshVersion getVersion() const { return _version; }
    /** @name Read API for membrane/surface mesh */
    //@{
    virtual size_t getNumVertices() const = 0;
    virtual Vector3fsPtr readVertices() const = 0;
    virtual uint16_tsPtr readVertexSections() const = 0;
    virtual floatsPtr readVertexDistances() const = 0;

    virtual size_t getNumTriangles() const = 0;
    virtual uint32_tsPtr readTriangles() const = 0;
    virtual uint16_tsPtr readTriangleSections() const = 0;
    virtual floatsPtr readTriangleDistances() const = 0;

    virtual size_t getTriStripLength() const = 0;
    virtual uint32_tsPtr readTriStrip() const = 0;

    virtual size_t getNumNormals() const = 0;
    virtual Vector3fsPtr readNormals() const = 0;
    //@}

    /** @name Read API for structural mesh */
    //@{
    virtual size_t getNumStructures(const MeshStructure type) const = 0;
    virtual Vector3fsPtr readStructureVertices(const MeshStructure type,
                                               const size_t index) const = 0;
    virtual uint32_tsPtr readStructureTriangles(const MeshStructure type,
                                                const size_t index) const = 0;
    virtual uint32_tsPtr readStructureTriStrip(const MeshStructure type,
                                               const size_t index) const = 0;
    //@}

    /** @name Write API for membrane/surface mesh */
    //@{
    virtual void writeVertices(const Vector3fs& vertices) = 0;
    virtual void writeVertexSections(const uint16_ts& vSections) = 0;
    virtual void writeVertexDistances(const floats& vDistances) = 0;

    virtual void writeTriangles(const uint32_ts& triangles) = 0;
    virtual void writeTriangleSections(const uint16_ts& tSections) = 0;
    virtual void writeTriangleDistances(const floats& tDistances) = 0;

    virtual void writeTriStrip(const uint32_ts& tristrip) = 0;

    virtual void writeNormals(const Vector3fs& normals) = 0;
    //@}

    /** @name Write API for structural mesh */
    //@{
    virtual void writeStructureVertices(const Vector3fs& vertices,
                                        const MeshStructure type,
                                        const size_t index) = 0;
    virtual void writeStructureTriangles(const uint32_ts& triangles,
                                         const MeshStructure type,
                                         const size_t index) = 0;
    virtual void writeStructureTriStrip(const uint32_ts& tristrip,
                                        const MeshStructure type,
                                        const size_t index) = 0;
    virtual void flush() = 0;
    //@}
};
}
}

#endif
