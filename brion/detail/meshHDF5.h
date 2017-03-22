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

#ifndef BRION_DETAIL_MESHHDF5
#define BRION_DETAIL_MESHHDF5

#include "mesh.h"
#include "silenceHDF5.h"

#include <H5Cpp.h>
#include <boost/lexical_cast.hpp>
#include <lunchbox/debug.h>

namespace boost
{
template <>
inline std::string lexical_cast(const brion::MeshStructure& f)
{
    switch (f)
    {
    case brion::MESH_SPINES:
        return "spines";
    case brion::MESH_BOUTONS:
        return "boutons";
    case brion::MESH_PSD:
        return "post_synaptic_densities";
    case brion::MESH_AZ:
        return "active_zones";
    case brion::MESH_ENDO:
        return "endoplasmic_reticulum";
    default:
        throw bad_lexical_cast();
    }
}
}

namespace brion
{
namespace detail
{
class MeshHDF5 : public Mesh
{
public:
    explicit MeshHDF5(const std::string& source)
        : Mesh(source)
        , _file(source, H5F_ACC_RDONLY)
    {
        try
        {
            SilenceHDF5 silence;
            _file.openDataSet("/membrane/mesh/vertices");
        }
        catch (...)
        {
            LBTHROW(std::runtime_error(source + " not a valid mesh file"));
        }
    }

    MeshHDF5(const std::string& source, const bool overwrite,
             const MeshVersion version)
        : Mesh(source, version)
        , _file()
    {
        try
        {
            SilenceHDF5 silence;
            _file =
                H5::H5File(source, overwrite ? H5F_ACC_TRUNC : H5F_ACC_EXCL);
        }
        catch (const H5::Exception& exc)
        {
            LBTHROW(std::runtime_error("Could not create mesh file " + source +
                                       ": " + exc.getDetailMsg()));
        }
    }

    virtual size_t getNumVertices() const
    {
        const H5::DataSet& dataset =
            _file.openDataSet("/membrane/mesh/vertices");
        return _numElements(dataset);
    }

    virtual Vector3fsPtr readVertices() const
    {
        const H5::DataSet& dataset =
            _file.openDataSet("/membrane/mesh/vertices");
        Vector3fsPtr buffer(new Vector3fs);
        buffer->resize(_numElements(dataset));
        dataset.read(buffer->data(), H5::PredType::IEEE_F32LE);
        return buffer;
    }

    virtual uint16_tsPtr readVertexSections() const
    {
        const H5::DataSet& dataset =
            _file.openDataSet("/membrane/mesh/mappings/vertex/section_ids");
        uint16_tsPtr buffer(new uint16_ts);
        buffer->resize(_numElements(dataset));
        dataset.read(buffer->data(), H5::PredType::STD_U32LE);
        return buffer;
    }

    virtual floatsPtr readVertexDistances() const
    {
        const H5::DataSet& dataset = _file.openDataSet(
            "/membrane/mesh/mappings/vertex/relative_positions");
        floatsPtr buffer(new floats);
        buffer->resize(_numElements(dataset));
        dataset.read(buffer->data(), H5::PredType::IEEE_F32LE);
        return buffer;
    }

    virtual size_t getNumTriangles() const
    {
        const H5::DataSet& dataset =
            _file.openDataSet("/membrane/mesh/triangles/soup");
        return _numElements(dataset);
    }

    virtual uint32_tsPtr readTriangles() const
    {
        const H5::DataSet& dataset =
            _file.openDataSet("/membrane/mesh/triangles/soup");
        uint32_tsPtr buffer(new uint32_ts);
        buffer->resize(_numElements(dataset) * 3);
        dataset.read(buffer->data(), H5::PredType::STD_U32LE);
        return buffer;
    }

    virtual uint16_tsPtr readTriangleSections() const
    {
        const H5::DataSet& dataset =
            _file.openDataSet("/membrane/mesh/mappings/triangle/section_ids");
        uint16_tsPtr buffer(new uint16_ts);
        buffer->resize(_numElements(dataset));
        dataset.read(buffer->data(), H5::PredType::STD_U32LE);
        return buffer;
    }

    virtual floatsPtr readTriangleDistances() const
    {
        const H5::DataSet& dataset = _file.openDataSet(
            "/membrane/mesh/mappings/triangle/relative_positions");
        floatsPtr buffer(new floats);
        buffer->resize(_numElements(dataset));
        dataset.read(buffer->data(), H5::PredType::IEEE_F32LE);
        return buffer;
    }

    virtual size_t getTriStripLength() const
    {
        const H5::DataSet& dataset =
            _file.openDataSet("/membrane/mesh/triangles/strip");
        return _numElements(dataset);
    }

    virtual uint32_tsPtr readTriStrip() const
    {
        uint32_tsPtr buffer(new uint32_ts);
        try
        {
            const H5::DataSet& dataset =
                _file.openDataSet("/membrane/mesh/triangles/strip");
            buffer->resize(_numElements(dataset));
            dataset.read(buffer->data(), H5::PredType::STD_U32LE);
        }
        catch (...)
        {
            LBINFO << "No tristrip in " << _file.getFileName() << std::endl;
        }

        return buffer;
    }

    virtual size_t getNumNormals() const
    {
        const H5::DataSet& dataset =
            _file.openDataSet("/membrane/mesh/normals");
        return _numElements(dataset);
    }

    virtual Vector3fsPtr readNormals() const
    {
        const H5::DataSet& dataset =
            _file.openDataSet("/membrane/mesh/normals");
        Vector3fsPtr buffer(new Vector3fs);
        buffer->resize(_numElements(dataset));
        dataset.read(buffer->data(), H5::PredType::IEEE_F32LE);
        return buffer;
    }

    virtual size_t getNumStructures(const MeshStructure type) const
    {
        // TODO: consider /structures group
        const H5::Group& group = _getStructureMappingGroup(type);
        return group.getNumObjs();
    }

    virtual Vector3fsPtr readStructureVertices(const MeshStructure type,
                                               const size_t index) const
    {
        // TODO: consider /structures group
        const H5::DataSet& dataset = _getStructureMapping(type, index);
        Vector3fsPtr buffer(new Vector3fs);
        buffer->resize(_numElements(dataset));
        dataset.read(buffer->data(), H5::PredType::IEEE_F32LE);
        return buffer;
    }

    virtual uint32_tsPtr readStructureTriangles(const MeshStructure type,
                                                const size_t index) const
    {
        // TODO: consider /structures group
        const H5::DataSet& dataset = _getStructureMapping(type, index);
        uint32_tsPtr buffer(new uint32_ts);
        buffer->resize(_numElements(dataset));
        dataset.read(buffer->data(), H5::PredType::STD_U32LE);
        return buffer;
    }

    virtual uint32_tsPtr readStructureTriStrip(const MeshStructure type,
                                               const size_t index) const
    {
        // TODO: consider /structures group
        const H5::DataSet& dataset = _getStructureMapping(type, index);
        uint32_tsPtr buffer(new uint32_ts);
        buffer->resize(_numElements(dataset));
        dataset.read(buffer->data(), H5::PredType::STD_U32LE);
        return buffer;
    }

    virtual void writeVertices(const Vector3fs& /*vertices*/)
    {
        LBUNIMPLEMENTED;
    }

    virtual void writeVertexSections(const uint16_ts& /*vSections*/)
    {
        LBUNIMPLEMENTED;
    }

    virtual void writeVertexDistances(const floats& /*vDistances*/)
    {
        LBUNIMPLEMENTED;
    }

    virtual void writeTriangles(const uint32_ts& /*triangles*/)
    {
        LBUNIMPLEMENTED;
    }

    virtual void writeTriangleSections(const uint16_ts& /*tSections*/)
    {
        LBUNIMPLEMENTED;
    }

    virtual void writeTriangleDistances(const floats& /*tDistances*/)
    {
        LBUNIMPLEMENTED;
    }

    virtual void writeTriStrip(const uint32_ts& /*tristrip*/)
    {
        LBUNIMPLEMENTED;
    }

    virtual void writeNormals(const Vector3fs& /*normals*/) { LBUNIMPLEMENTED; }
    virtual void writeStructureVertices(const Vector3fs& /*vertices*/,
                                        const MeshStructure /*type*/,
                                        const size_t /*index*/)
    {
        LBUNIMPLEMENTED;
    }

    virtual void writeStructureTriangles(const uint32_ts& /*triangles*/,
                                         const MeshStructure /*type*/,
                                         const size_t /*index*/)
    {
        LBUNIMPLEMENTED;
    }

    virtual void writeStructureTriStrip(const uint32_ts& /*tristrip*/,
                                        const MeshStructure /*type*/,
                                        const size_t /*index*/)
    {
        LBUNIMPLEMENTED;
    }

    virtual void flush() { LBUNIMPLEMENTED; }
private:
    hsize_t _numElements(const H5::DataSet& dataset) const
    {
        hsize_t dims[2];
        dataset.getSpace().getSimpleExtentDims(dims);
        return dims[0];
    }

    H5::Group _getStructureMappingGroup(const MeshStructure type) const
    {
        return _file.openGroup("/membrane/mesh/mappings/structure/" +
                               boost::lexical_cast<std::string>(type));
    }

    H5::DataSet _getStructureMapping(const MeshStructure type,
                                     const size_t index) const
    {
        const H5::Group& group = _getStructureMappingGroup(type);
        return _file.openDataSet(group.getObjnameByIdx(index));
    }

    H5::H5File _file;
};
}
}

#endif
