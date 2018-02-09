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

#include "soma.h"

namespace minimorph
{
Soma::Soma(PropertiesPtr properties)
    : _properties(properties)
{
    uint32_t id = 0;
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
        LBWARN << "Dereferencing broken soma "
               << "Section range: " << _range.first << " -> " << _range.second
               << std::endl;
}

SectionType Soma::getType(){
    return get<Property::SectionType>()[0];
}

template <typename TProperty> const typename TProperty::Type Soma::get() const
{
    const auto& data = _properties->get<TProperty>();
    typename TProperty::Type result;
    result.reserve(_range.second - _range.first);
    result.insert(result.end(), data.begin() + _range.first,
                  data.begin() + _range.second);
    return result;
}

Point Soma::getSomaCenter()
{
    auto points = get<Property::Point>();
    float x = 0, y = 0, z = 0;
    float size = float(points.size());
    for (const Point& point : points)
    {
        x += point[0];
        y += point[1];
        z += point[2];
    }
    return Point({x/size, y/size, z/size});
}

template const Property::Point::Type Soma::get<Property::Point>() const;
template const Property::Diameter::Type Soma::get<Property::Diameter>() const;
}
