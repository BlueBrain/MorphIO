/* Copyright (c) 2013-2016, EPFL/Blue Brain Project
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
#include "morphologyImpl.h"
#include "section.h"

namespace brain
{
namespace neuron
{
namespace
{
Vector3f _computeCentroid(const Vector4fs& points)
{
    Vector3f centroid;
    for (const Vector4f& point : points)
        centroid += point.get_sub_vector<3, 0>();
    centroid /= float(points.size());
    return centroid;
}
}

Soma::Soma(Morphology::Impl* morphology)
    : _morphology(morphology)
{
    _morphology->ref();
}

Soma::~Soma()
{
    _morphology->unref();
}

Soma::Soma(const Soma& soma)
    : _morphology(soma._morphology)
{
    _morphology->ref();
}

Soma& Soma::operator=(const Soma& soma)
{
    if (&soma == this)
        return *this;
    if (_morphology)
        _morphology->unref();
    _morphology = soma._morphology;
    _morphology->ref();
    return *this;
}

Vector4fs Soma::getProfilePoints() const
{
    return _morphology->getSectionSamples(_morphology->somaSection);
}

float Soma::getMeanRadius() const
{
    const Vector4fs points = getProfilePoints();
    const Vector3f centroid = _computeCentroid(points);
    float radius = 0;
    for (const Vector4f point : points)
        radius += (point.get_sub_vector<3, 0>() - centroid).length();
    return radius /= float(points.size());
}

Vector3f Soma::getCentroid() const
{
    return _computeCentroid(getProfilePoints());
}

Sections Soma::getChildren() const
{
    const uint32_ts& children =
        _morphology->getChildren(_morphology->somaSection);
    Sections result;
    for (uint32_t id : children)
        result.push_back(Section(id, _morphology));
    return result;
}
}
}
