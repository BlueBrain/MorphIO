/* Copyright (c) 2013-2016, EPFL/Blue Brain Project
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

#ifndef BRION_DETAIL_MESHBINARY
#define BRION_DETAIL_MESHBINARY

#include "mesh.h"

#include <fstream>
#include <lunchbox/debug.h>
#include <lunchbox/log.h>
#include <lunchbox/memoryMap.h>
#include <vmmlib/vector.hpp>

namespace brion
{
namespace detail
{
template <typename T>
T get(const uint8_t* buffer, size_t& pos)
{
    const T* val = reinterpret_cast<const T*>(buffer + pos);
    pos += sizeof(T);
    return *val;
}

class MeshBinary : public Mesh
{
public:
    explicit MeshBinary(const std::string& source)
        : Mesh(source)
        , _mmap(source)
        , _ptr(reinterpret_cast<const uint8_t*>(_mmap.getAddress()))
    {
        if (!_ptr)
            LBTHROW(std::runtime_error("Could not open mesh file: " + source));

        size_t pos = 0;
        _vertices = get<uint32_t>(_ptr, pos);
        _triangles = get<uint32_t>(_ptr, pos);
        _tristrip = get<uint32_t>(_ptr, pos);

        _vertexSeek = sizeof(uint32_t) * 3;
        _vSectionSeek = _vertexSeek + _vertices * 3 * sizeof(float);
        _vDistanceSeek = _vSectionSeek + _vertices * sizeof(uint16_t);
        _triangleSeek = _vDistanceSeek + _vertices * sizeof(float);
        _tristripSeek = _triangleSeek + _triangles * 3 * sizeof(uint32_t);

        // if the version is contained in the current file apply offset
        if (_mmap.getSize() != _tristripSeek + _tristrip * sizeof(uint32_t))
        {
            _version = get<MeshVersion>(_ptr, pos);

            const size_t versionOffset = sizeof(_version);
            _vertexSeek += versionOffset;
            _vSectionSeek += versionOffset;
            _vDistanceSeek += versionOffset;
            _triangleSeek += versionOffset;
            _tristripSeek += versionOffset;
        }
    }

    MeshBinary(const std::string& source, const MeshVersion version)
        : Mesh(source, version)
        , _mmap()
        , _ptr()
        , _file(source.c_str(), std::ios::binary | std::ios::trunc)
        , _vertices(0)
        , _triangles(0)
        , _tristrip(0)
        , _vSectionSeek(0)
        , _vDistanceSeek(0)
        , _triangleSeek(0)
        , _tristripSeek(0)
    {
        if (!_file.is_open())
            LBTHROW(std::runtime_error("Could not open mesh file " + source +
                                       " for writing "));

        _vertexSeek = sizeof(uint32_t) * 4;

        _file.seekp(3 * sizeof(uint32_t));
        _file.write((const char*)&_version, sizeof(_version));
    }

    virtual size_t getNumVertices() const { return _vertices; }
    virtual Vector3fsPtr readVertices() const
    {
        Vector3fsPtr buffer(new Vector3fs);
        if (!_ptr)
            return buffer;

        buffer->resize(_vertices);

        memcpy(buffer->data(), _ptr + _vertexSeek,
               _vertices * 3 * sizeof(float));
        return buffer;
    }

    virtual uint16_tsPtr readVertexSections() const
    {
        uint16_tsPtr buffer(new uint16_ts);
        if (!_ptr)
            return buffer;

        buffer->resize(_vertices);

        memcpy(buffer->data(), _ptr + _vSectionSeek,
               _vertices * sizeof(uint16_t));
        return buffer;
    }

    virtual floatsPtr readVertexDistances() const
    {
        floatsPtr buffer(new floats);
        if (!_ptr)
            return buffer;

        buffer->resize(_vertices);

        memcpy(buffer->data(), _ptr + _vDistanceSeek,
               _vertices * sizeof(float));
        return buffer;
    }

    virtual size_t getNumTriangles() const { return _triangles; }
    virtual uint32_tsPtr readTriangles() const
    {
        uint32_tsPtr buffer(new uint32_ts);
        if (!_ptr)
            return buffer;

        buffer->resize(_triangles * 3);

        memcpy(buffer->data(), _ptr + _triangleSeek,
               _triangles * 3 * sizeof(uint32_t));
        return buffer;
    }

    virtual uint16_tsPtr readTriangleSections() const
    {
        return uint16_tsPtr(new uint16_ts);
    }

    virtual floatsPtr readTriangleDistances() const
    {
        return floatsPtr(new floats);
    }

    virtual size_t getTriStripLength() const { return _tristrip; }
    virtual uint32_tsPtr readTriStrip() const
    {
        uint32_tsPtr buffer(new uint32_ts);
        if (!_ptr)
            return buffer;

        buffer->resize(_tristrip);

        memcpy(buffer->data(), _ptr + _tristripSeek,
               _tristrip * sizeof(uint32_t));
        return buffer;
    }

    virtual size_t getNumNormals() const { return 0u; }
    virtual Vector3fsPtr readNormals() const
    {
        return Vector3fsPtr(new Vector3fs);
    }

    virtual size_t getNumStructures(const MeshStructure /*type*/) const
    {
        return 0u;
    }

    virtual Vector3fsPtr readStructureVertices(const MeshStructure /*type*/,
                                               const size_t /*index*/) const
    {
        return Vector3fsPtr(new Vector3fs);
    }

    virtual uint32_tsPtr readStructureTriangles(const MeshStructure /*type*/,
                                                const size_t /*index*/) const
    {
        return uint32_tsPtr(new uint32_ts);
    }

    virtual uint32_tsPtr readStructureTriStrip(const MeshStructure /*type*/,
                                               const size_t /*index*/) const
    {
        return uint32_tsPtr(new uint32_ts);
    }

    virtual void writeVertices(const Vector3fs& vertices)
    {
        _vertices = uint32_t(vertices.size());

        // initialize all seek positions depending on vertices
        _vSectionSeek = _vertexSeek + _vertices * 3 * sizeof(float);
        _vDistanceSeek = _vSectionSeek + _vertices * sizeof(uint16_t);
        _triangleSeek = _vDistanceSeek + _vertices * sizeof(float);

        _file.seekp(0);
        _file.write((const char*)&_vertices, sizeof(_vertices));
        _file.seekp(_vertexSeek);
        _file.write((const char*)vertices.data(),
                    vertices.size() * 3 * sizeof(float));
    }

    virtual void writeVertexSections(const uint16_ts& vSections)
    {
        if (_vertices != vSections.size())
            LBTHROW(
                std::runtime_error("Number of vertices does not match "
                                   "number of vertex sections"));
        _file.seekp(_vSectionSeek);
        _file.write((const char*)vSections.data(),
                    vSections.size() * sizeof(uint16_t));
    }

    virtual void writeVertexDistances(const floats& vDistances)
    {
        if (_vertices != vDistances.size())
            LBTHROW(
                std::runtime_error("Number of vertices does not match "
                                   "number of vertex distances"));
        _file.seekp(_vDistanceSeek);
        _file.write((const char*)vDistances.data(),
                    vDistances.size() * sizeof(float));
    }

    virtual void writeTriangles(const uint32_ts& triangles)
    {
        if (_vertices == 0)
            LBTHROW(
                std::runtime_error("No vertices written before "
                                   "triangles"));
        LBASSERT(triangles.size() % 3 == 0);
        _triangles = uint32_t(triangles.size() / 3);

        // initialize seek position of triangle strip depending on triangles
        _tristripSeek = _triangleSeek + _triangles * 3 * sizeof(uint32_t);

        _file.seekp(sizeof(uint32_t));
        _file.write((const char*)&_triangles, sizeof(_triangles));
        _file.seekp(_triangleSeek);
        _file.write((const char*)triangles.data(),
                    triangles.size() * sizeof(uint32_t));
    }

    virtual void writeTriangleSections(const uint16_ts& /*tSections*/)
    {
        LBTHROW(
            std::runtime_error("No triangle sections support for binary "
                               "mesh files"));
    }

    virtual void writeTriangleDistances(const floats& /*tDistances*/)
    {
        LBTHROW(
            std::runtime_error("No triangle distances support for binary "
                               "mesh files"));
    }

    virtual void writeTriStrip(const uint32_ts& tristrip)
    {
        if (_vertices == 0)
            LBTHROW(
                std::runtime_error("No vertices written before "
                                   "tristrip"));

        _tristrip = uint32_t(tristrip.size());
        _file.seekp(2 * sizeof(uint32_t));
        _file.write((const char*)&_tristrip, sizeof(_tristrip));
        _file.seekp(_tristripSeek);
        _file.write((const char*)tristrip.data(),
                    tristrip.size() * sizeof(uint32_t));
    }

    virtual void writeNormals(const Vector3fs& /*normals*/)
    {
        LBTHROW(
            std::runtime_error("No normal support for binary mesh "
                               "files"));
    }

    virtual void writeStructureVertices(const Vector3fs& /*vertices*/,
                                        const MeshStructure /*type*/,
                                        const size_t /*index*/)
    {
        LBTHROW(
            std::runtime_error("No structural mesh support for binary "
                               "mesh files"));
    }

    virtual void writeStructureTriangles(const uint32_ts& /*triangles*/,
                                         const MeshStructure /*type*/,
                                         const size_t /*index*/)
    {
        LBTHROW(
            std::runtime_error("No structural mesh support for binary "
                               "mesh files"));
    }

    virtual void writeStructureTriStrip(const uint32_ts& /*tristrip*/,
                                        const MeshStructure /*type*/,
                                        const size_t /*index*/)
    {
        LBTHROW(
            std::runtime_error("No structural mesh support for binary "
                               "mesh files"));
    }

    virtual void flush() { _file.flush(); }
private:
    lunchbox::MemoryMap _mmap;
    const uint8_t* const _ptr;
    std::ofstream _file;

    uint32_t _vertices;
    uint32_t _triangles;
    uint32_t _tristrip;

    size_t _vertexSeek;
    size_t _vSectionSeek;
    size_t _vDistanceSeek;
    size_t _triangleSeek;
    size_t _tristripSeek;
};
}
}

#endif
