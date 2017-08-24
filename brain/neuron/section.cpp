
/* Copyright (c) 2013-2017, EPFL/Blue Brain Project
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

#include "section.h"

#include "morphology.h"
#include "morphologyImpl.h"

namespace brain
{
namespace neuron
{
Section::Section(const uint32_t id, Morphology::ImplPtr morphology)
    : _id(id)
    , _morphology(morphology)
{
    SectionRange range = morphology->getSectionRange(id);
    if (range.second <= range.first)
        LBWARN << "Dereferencing broken morphology section " << _id
               << std::endl;
}

Section::Section(const Section& section)
    : _id(section._id)
    , _morphology(section._morphology)
{
}

Section& Section::operator=(const Section& section)
{
    if (&section == this)
        return *this;
    _id = section._id;
    _morphology = section._morphology;
    return *this;
}

bool Section::operator==(const Section& other) const
{
    return other._id == _id && other._morphology == _morphology;
}

bool Section::operator!=(const Section& other) const
{
    return !(*this == other);
}
uint32_t Section::getID() const
{
    return _id;
}

SectionType Section::getType() const
{
    return static_cast<SectionType>(_morphology->data->getSectionTypes()[_id]);
}

float Section::getLength() const
{
    return _morphology->getSectionLength(_id);
}

Vector4fs Section::getSamples() const
{
    return _morphology->getSectionSamples(_id);
}

Vector4fs Section::getSamples(const floats& points) const
{
    return _morphology->getSectionSamples(_id, points);
}

float Section::getDistanceToSoma() const
{
    return _morphology->getDistanceToSoma(_id);
}

floats Section::getSampleDistancesToSoma() const
{
    return _morphology->getSampleDistancesToSoma(_id);
}

bool Section::hasParent() const
{
    const int32_t parent = _morphology->data->getSections()[_id][1];
    return parent != -1 && uint32_t(parent) != _morphology->somaSection;
}
Section Section::getParent() const
{
    const int32_t parent = _morphology->data->getSections()[_id][1];
    if (parent == -1 || uint32_t(parent) == _morphology->somaSection)
        LBTHROW(std::runtime_error("Cannot access parent section"));
    return Section(parent, _morphology);
}

Sections Section::getChildren() const
{
    const uint32_ts& children = _morphology->getChildren(_id);
    Sections result;
    result.reserve(children.size());
    for (const uint32_t id : children)
        result.push_back(Section(id, _morphology));
    return result;
}
}
}
