/* Copyright (c) 2013-2017, EPFL/Blue Brain Project
 *                          Juan Hernando <jhernando@fi.upm.es>
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

#ifndef BRION_PLUGIN_MORPHOLOGYSWC
#define BRION_PLUGIN_MORPHOLOGYSWC

#include "../morphologyPlugin.h"

namespace brion
{
namespace plugin
{
class MorphologySWC : public MorphologyPlugin
{
public:
    explicit MorphologySWC(const MorphologyInitData& initData);

    /** Check if this plugin can handle the given uri. */
    static bool handles(const MorphologyInitData& initData);
    static std::string getDescription();

    Vector4fsPtr readPoints(MorphologyRepairStage stage) const final;

    Vector2isPtr readSections(MorphologyRepairStage stage) const final;

    SectionTypesPtr readSectionTypes() const final;

    Vector2isPtr readApicals() const final;

    floatsPtr readPerimeters() const final;

    void writePoints(const Vector4fs& points,
                     MorphologyRepairStage stage) final;

    void writeSections(const Vector2is& sections,
                       MorphologyRepairStage stage) final;

    void writeSectionTypes(const SectionTypes& types) final;

    void writeApicals(const Vector2is& apicals) final;

    void writePerimeters(const floats& perimeters) final;

    void flush() final;

private:
    struct RawSWCInfo;

    Vector4fsPtr _points;
    Vector2isPtr _sections;
    SectionTypesPtr _types;

    void _readSamples(RawSWCInfo& info);

    void _buildSampleTree(RawSWCInfo& info);

    void _buildStructure(RawSWCInfo& info);
};
}
}
#endif
