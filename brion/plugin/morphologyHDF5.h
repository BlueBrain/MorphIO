/* Copyright (c) 2013-2017, EPFL/Blue Brain Project
 *                          Daniel Nachbaur <daniel.nachbaur@epfl.ch>
 *                          Juan Hernando <jhernando@fi.upm.es>
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

#ifndef BRION_PLUGIN_MORPHOLOGYHDF5
#define BRION_PLUGIN_MORPHOLOGYHDF5

#include "../morphologyPlugin.h"

#include <H5Cpp.h>

namespace brion
{
namespace plugin
{
class MorphologyHDF5 : public MorphologyPlugin
{
public:
    /** Create a new parser for an H5 morphology */
    explicit MorphologyHDF5(const MorphologyInitData& initData);

    ~MorphologyHDF5();

    /** Check if this plugin can handle the given uri. */
    static bool handles(const MorphologyInitData& initData);
    static std::string getDescription();

    CellFamily getCellFamily() const final;

    Vector4fsPtr readPoints(MorphologyRepairStage stage) const final;

    Vector2isPtr readSections(MorphologyRepairStage stage) const final;

    SectionTypesPtr readSectionTypes() const final;

    Vector2isPtr readApicals() const final;

    floatsPtr readPerimeters() const final;

    MorphologyVersion getVersion() const final;

    void writePoints(const Vector4fs& points,
                     MorphologyRepairStage stage) final;

    void writeSections(const Vector2is& sections,
                       MorphologyRepairStage stage) final;

    void writeSectionTypes(const SectionTypes& types) final;

    void writeApicals(const Vector2is& apicals) final;

    void writePerimeters(const floats& perimeters) final;

    void flush() final;

private:
    H5::H5File _file;

    H5::DataSet _points;
    hsize_t _pointsDims[2];

    H5::DataSet _sections;
    hsize_t _sectionsDims[2];

    MorphologyVersion _version;
    MorphologyRepairStage _stage;
    CellFamily _family;
    bool _write;

    void _checkVersion(const std::string& source);
    void _selectRepairStage();

    void _resolveV1();

    void _writeV11Metadata(const MorphologyInitData& initData);
    bool _readV11Metadata();

    bool _readV2Metadata();
    void _writeV2Metadata();

    H5::DataSet _getStructureDataSet(size_t nSections);
};
}
}

#endif
