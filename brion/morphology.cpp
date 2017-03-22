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

#include "morphology.h"

#include "morphologyPlugin.h"

#include <lunchbox/plugin.h>
#include <lunchbox/pluginFactory.h>

#include <boost/scoped_ptr.hpp>

namespace brion
{
class Morphology::Impl
{
public:
    typedef lunchbox::PluginFactory<MorphologyPlugin> MorphologyPluginFactory;

    explicit Impl(const MorphologyInitData& initData)
        : plugin(MorphologyPluginFactory::getInstance().create(initData))
    {
    }

    boost::scoped_ptr<MorphologyPlugin> plugin;
};

Morphology::Morphology(const std::string& source)
    : _impl(new Impl(MorphologyInitData(URI(source))))
{
}

Morphology::Morphology(const std::string& target,
                       const MorphologyVersion version, const bool overwrite)
    : _impl(
          new Impl(MorphologyInitData(URI(target), version,
                                      overwrite ? MODE_OVERWRITE : MODE_WRITE)))
{
}

Morphology::Morphology(const std::string& file, const CellFamily family)
    : _impl(new Impl(MorphologyInitData(URI(file), family)))
{
}

Morphology::~Morphology()
{
    delete _impl;
}

CellFamily Morphology::getCellFamily() const
{
    return _impl->plugin->getCellFamily();
}

Vector4fsPtr Morphology::readPoints(const MorphologyRepairStage stage) const
{
    return _impl->plugin->readPoints(stage);
}

Vector2isPtr Morphology::readSections(const MorphologyRepairStage stage) const
{
    return _impl->plugin->readSections(stage);
}

SectionTypesPtr Morphology::readSectionTypes() const
{
    return _impl->plugin->readSectionTypes();
}

Vector2isPtr Morphology::readApicals() const
{
    return _impl->plugin->readApicals();
}

floatsPtr Morphology::readPerimeters() const
{
    return _impl->plugin->readPerimeters();
}

MorphologyVersion Morphology::getVersion() const
{
    return _impl->plugin->getVersion();
}

void Morphology::writePoints(const Vector4fs& points,
                             const MorphologyRepairStage stage)
{
    _impl->plugin->writePoints(points, stage);
}

void Morphology::writeSections(const Vector2is& sections,
                               const MorphologyRepairStage stage)
{
    _impl->plugin->writeSections(sections, stage);
}

void Morphology::writeSectionTypes(const SectionTypes& types)
{
    _impl->plugin->writeSectionTypes(types);
}

void Morphology::writeApicals(const Vector2is& apicals)
{
    _impl->plugin->writeApicals(apicals);
}

void Morphology::writePerimeters(const floats& perimeters)
{
    _impl->plugin->writePerimeters(perimeters);
}

void Morphology::flush()
{
    _impl->plugin->flush();
}
}
