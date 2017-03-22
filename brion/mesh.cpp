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

#include "mesh.h"
#include "detail/meshBinary.h"
#include "detail/meshHDF5.h"

#include <boost/filesystem.hpp>

#define ASSERT_WRITE                                             \
    if (!_impl->_write)                                          \
        LBTHROW(                                                 \
            std::runtime_error("Cannot not write read-only mesh" \
                               " file " +                        \
                               _impl->_source));

namespace brion
{
Mesh::Mesh(const std::string& source)
    : _impl(0)
{
    namespace fs = boost::filesystem;
    fs::path path(source);
    const std::string& ext = fs::extension(path);
    if (ext == ".bin")
    {
        _impl = new detail::MeshBinary(source);
        return;
    }
    if (ext == ".h5" || ext == ".hdf5")
    {
        _impl = new detail::MeshHDF5(source);
        return;
    }
    LBTHROW(std::runtime_error(source + " not a valid mesh file"));
}

Mesh::Mesh(const std::string& source, const MeshFormat format,
           const bool overwrite, const MeshVersion version)
    : _impl(0)
{
    if (!overwrite && boost::filesystem::exists(source))
        LBTHROW(std::runtime_error("Cannot override existing file " + source));

    switch (format)
    {
    case MESHFORMAT_HDF5:
        _impl = new detail::MeshHDF5(source, overwrite, version);
        return;
    case MESHFORMAT_BINARY:
    default:
        _impl = new detail::MeshBinary(source, version);
    }
}

Mesh::~Mesh()
{
    delete _impl;
}

MeshVersion Mesh::getVersion() const
{
    return _impl->getVersion();
}

size_t Mesh::getNumVertices() const
{
    return _impl->getNumVertices();
}

Vector3fsPtr Mesh::readVertices() const
{
    return _impl->readVertices();
}

uint16_tsPtr Mesh::readVertexSections() const
{
    return _impl->readVertexSections();
}

floatsPtr Mesh::readVertexDistances() const
{
    return _impl->readVertexDistances();
}

size_t Mesh::getNumTriangles() const
{
    return _impl->getNumTriangles();
}

uint32_tsPtr Mesh::readTriangles() const
{
    return _impl->readTriangles();
}

uint16_tsPtr Mesh::readTriangleSections() const
{
    return _impl->readTriangleSections();
}

floatsPtr Mesh::readTriangleDistances() const
{
    return _impl->readTriangleDistances();
}

size_t Mesh::getTriStripLength() const
{
    return _impl->getTriStripLength();
}

uint32_tsPtr Mesh::readTriStrip() const
{
    return _impl->readTriStrip();
}

size_t Mesh::getNumNormals() const
{
    return _impl->getNumNormals();
}

Vector3fsPtr Mesh::readNormals() const
{
    return _impl->readNormals();
}

size_t Mesh::getNumStructures(const MeshStructure type) const
{
    return _impl->getNumStructures(type);
}

Vector3fsPtr Mesh::readStructureVertices(const MeshStructure type,
                                         const size_t index) const
{
    return _impl->readStructureVertices(type, index);
}

uint32_tsPtr Mesh::readStructureTriangles(const MeshStructure type,
                                          const size_t index) const
{
    return _impl->readStructureTriangles(type, index);
}

uint32_tsPtr Mesh::readStructureTriStrip(const MeshStructure type,
                                         const size_t index) const
{
    return _impl->readStructureTriStrip(type, index);
}

void Mesh::writeVertices(const Vector3fs& vertices)
{
    ASSERT_WRITE;
    _impl->writeVertices(vertices);
}

void Mesh::writeVertexSections(const uint16_ts& vSections)
{
    ASSERT_WRITE;
    _impl->writeVertexSections(vSections);
}

void Mesh::writeVertexDistances(const floats& vDistances)
{
    ASSERT_WRITE;
    _impl->writeVertexDistances(vDistances);
}

void Mesh::writeTriangles(const uint32_ts& triangles)
{
    ASSERT_WRITE;
    _impl->writeTriangles(triangles);
}

void Mesh::writeTriangleSections(const uint16_ts& tSections)
{
    ASSERT_WRITE;
    _impl->writeTriangleSections(tSections);
}

void Mesh::writeTriangleDistances(const floats& tDistances)
{
    ASSERT_WRITE;
    _impl->writeTriangleDistances(tDistances);
}

void Mesh::writeTriStrip(const uint32_ts& tristrip)
{
    ASSERT_WRITE;
    _impl->writeTriStrip(tristrip);
}

void Mesh::writeNormals(const Vector3fs& normals)
{
    ASSERT_WRITE;
    _impl->writeNormals(normals);
}

void Mesh::writeStructureVertices(const Vector3fs& vertices,
                                  const MeshStructure type, const size_t index)
{
    ASSERT_WRITE;
    _impl->writeStructureVertices(vertices, type, index);
}

void Mesh::writeStructureTriangles(const uint32_ts& triangles,
                                   const MeshStructure type, const size_t index)

{
    ASSERT_WRITE;
    _impl->writeStructureTriangles(triangles, type, index);
}

void Mesh::writeStructureTriStrip(const uint32_ts& tristrip,
                                  const MeshStructure type, const size_t index)

{
    ASSERT_WRITE;
    _impl->writeStructureTriStrip(tristrip, type, index);
}

void Mesh::flush()
{
    ASSERT_WRITE;
    _impl->flush();
}
}
