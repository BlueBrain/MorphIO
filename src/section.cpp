
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

namespace minimorph
{



Section::Section(const uint32_t id, PropertiesPtr properties)
    : _id(id)
    , _properties(properties)
{
    const auto& points = properties->get<Property::Point>();
    const auto& sections = properties->get<Property::Section>();
    if(id >= sections.size())
        LBTHROW(RawDataError("Requested section ID (" + std::to_string(id) + \
                             ") is out of array bounds (array size = " + \
                             std::to_string(sections.size()) + ")"));

    const size_t start = sections[id][0];
    const size_t end = id == sections.size() - 1
        ? points.size()
        : sections[id + 1][0];
    _range = std::make_pair(start, end);

    if (_range.second <= _range.first)
        LBWARN << "Dereferencing broken properties section " << _id << std:: endl
               << "Section range: " << _range.first << " -> " << _range.second
               << std::endl;
}

Section::Section(const Section& section)
    : _id(section._id)
    , _range(section._range)
    , _properties(section._properties)
{
}

Section& Section::operator=(const Section& section)
{
    if (&section == this)
        return *this;
    _id = section._id;
    _range = section._range;
    _properties = section._properties;
    return *this;
}

bool Section::operator==(const Section& other) const
{
    return other._id == _id && other._properties == _properties;
}

bool Section::operator!=(const Section& other) const
{
    return !(*this == other);
}
uint32_t Section::getID() const
{
    return _id;
}

const SectionType Section::getType() const
{
    return get<Property::SectionType>()[_id];
}


template <typename TProperty> const typename TProperty::Type Section::get() const
{
    const auto& data = _properties->get<TProperty>();
    typename TProperty::Type result;
    result.reserve(_range.second - _range.first);
    result.insert(result.end(), data.begin() + _range.first,
                  data.begin() + _range.second);
    return result;
}

std::shared_ptr<Section> Section::getParent() const
{
    const int32_t parent = _properties->get<Property::Section>()[_id][1];
// return (parent > -1) ? std::experimental::optional<Section>(Section(parent, _properties)) : std::experimental::nullopt;
    return (parent > -1) ? std::make_shared<Section>(Section(parent, _properties)) : nullptr;
}

Sections Section::getChildren() const
{
    const uint32_ts& children = _properties->get<Property::Children>()[_id];
    Sections result;
    result.reserve(children.size());
    for (const uint32_t id : children)
        result.push_back(Section(id, _properties));
    return result;
}



depth_iterator Section::depth_begin() {
    return depth_iterator(*this);
}

depth_iterator Section::depth_end() {
    return depth_iterator();
}

breadth_iterator Section::breadth_begin() {
    return breadth_iterator(*this);
}

breadth_iterator Section::breadth_end() {
    return breadth_iterator();
}

upstream_iterator Section::upstream_begin() {
    return upstream_iterator(*this);
}

upstream_iterator Section::upstream_end() {
    return upstream_iterator();
}


std::ostream& operator<<(std::ostream& os, const Section& section){
    os << section.getID();
    return os;
}

const Points Section::getPoints() const { return get<Property::Point>(); }
const floats Section::getDiameters() const { return get<Property::Diameter>(); }
const floats Section::getPerimeters() const { return get<Property::Perimeter>(); }

}
