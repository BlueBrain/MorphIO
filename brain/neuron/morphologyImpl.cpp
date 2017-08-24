
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

#include "morphologyImpl.h"
#include "section.h"

#include <brion/morphology.h>
#include <brion/morphologyPlugin.h>

#include <lunchbox/log.h>

#include <bitset>

namespace brain
{
namespace neuron
{
Morphology::Impl::Impl(const void* ptr, const size_t size)
    : Impl(brion::ConstMorphologyPtr(new brion::Morphology(ptr, size)))
{
}

Morphology::Impl::Impl(const URI& source)
    : Impl(brion::ConstMorphologyPtr(new brion::Morphology(source)))
{
}

Morphology::Impl::Impl(const URI& source, const Matrix4f& transform)
    : Impl(brion::MorphologyPtr(new brion::Morphology(source)), transform)
{
}

Morphology::Impl::Impl(brion::ConstMorphologyPtr morphology)
    : data(morphology)
{
    _extractInformation();
}

Morphology::Impl::Impl(brion::MorphologyPtr morphology,
                       const Matrix4f& transform)
    : data(morphology)
    , transformation(transform)

{
    _transform(morphology);
    _extractInformation();
}

SectionRange Morphology::Impl::getSectionRange(const uint32_t sectionID) const
{
    const auto& points = data->getPoints();
    const auto& sections = data->getSections();
    const size_t start = sections[sectionID][0];
    const size_t end = sectionID == sections.size() - 1
                           ? points.size()
                           : sections[sectionID + 1][0];
    return std::make_pair(start, end);
}

uint32_ts Morphology::Impl::getSectionIDs(const SectionTypes& requestedTypes,
                                          const bool excludeSoma) const
{
    std::bitset<size_t(SectionType::apicalDendrite)> bits;
    for (const SectionType type : requestedTypes)
    {
        if (type != SectionType::soma || !excludeSoma)
            bits[size_t(type)] = true;
    }

    uint32_ts result;
    const auto& types = data->getSectionTypes();
    for (size_t i = 0; i != types.size(); ++i)
    {
        const SectionType type = static_cast<SectionType>(types[i]);
        if (bits[size_t(type)])
            result.push_back(i);
    }
    return result;
}

float Morphology::Impl::getSectionLength(const uint32_t sectionID) const
{
    if (_sectionLengths.size() <= sectionID)
        _sectionLengths.resize(sectionID + 1);

    float& length = _sectionLengths[sectionID];
    const auto& types = data->getSectionTypes();

    if (length == 0 && types[sectionID] != brion::enums::SECTION_SOMA)
        length = _computeSectionLength(sectionID);
    return length;
}

Vector4fs Morphology::Impl::getSectionSamples(const uint32_t sectionID) const
{
    const SectionRange range = getSectionRange(sectionID);
    const auto& points = data->getPoints();

    Vector4fs result;
    result.reserve(range.second - range.first);
    result.insert(result.end(), points.begin() + range.first,
                  points.begin() + range.second);
    return result;
}

Vector4fs Morphology::Impl::getSectionSamples(const uint32_t sectionID,
                                              const floats& samplePoints) const
{
    const SectionRange range = getSectionRange(sectionID);
    const auto& types = data->getSectionTypes();

    // If the section is the soma return directly the soma position.
    if (types[sectionID] == brion::enums::SECTION_SOMA)
        // This code shouldn't be reached.
        LBTHROW(std::runtime_error("Invalid method called on soma section"));

    // Dealing with the degenerate case of single point sections.
    const auto& points = data->getPoints();
    if (range.first + 1 == range.second)
        return Vector4fs(samplePoints.size(), points[range.first]);

    Vector4fs result;
    result.reserve(samplePoints.size());

    const floats accumLengths = _computeAccumulatedLengths(range);
    const float totalLength = accumLengths.back();

    for (const float point : samplePoints)
    {
        // Finding the segment index for the requested sampling position.
        const float length = std::max(0.f, std::min(1.f, point)) * totalLength;
        size_t index = 0;
        for (; accumLengths[index + 1] < length &&
               index < accumLengths.size() - 1;
             ++index)
            ;

        // If the first point of the section is repeated and we are
        // interpolating
        // at 0 length - accumLengths[0] and accumLengths[1] - accumLengths[0]
        // will be both 0. To avoid the 0/0 operation we check for
        // length == accumLengths[index].
        const size_t start = range.first + index;
        if (length == accumLengths[index])
        {
            result.push_back(points[start]);
            continue;
        }

        // Interpolating the cross section at point.
        const float alpha = (length - accumLengths[index]) /
                            (accumLengths[index + 1] - accumLengths[index]);
        const Vector4f sample =
            points[start + 1] * alpha + points[start] * (1 - alpha);
        result.push_back(sample);
    }

    return result;
}

float Morphology::Impl::getDistanceToSoma(const uint32_t sectionID) const
{
    if (_distancesToSoma.size() <= sectionID)
        _distancesToSoma.resize(sectionID + 1);

    float& distance = _distancesToSoma[sectionID];
    if (distance == 0)
    {
        // This is the soma, a first order section or the distance hasn't
        // been computed yet. Soma and first order sections are cheap
        // to detect and compute.
        const auto& sections = data->getSections();
        const auto& types = data->getSectionTypes();
        const int32_t parent = sections[sectionID][1];
        if (parent == -1 || types[parent] == brion::enums::SECTION_SOMA)
            return 0;
        // For the other cases it doesn't matter to have concurrent updates
        // because they will yield the same result (and it's probably
        // cheaper to go ahead with the computation than to contend for a
        // mutex).
        distance = getSectionLength(parent) + getDistanceToSoma(parent);
    }
    return distance;
}

floats Morphology::Impl::getSampleDistancesToSoma(
    const uint32_t sectionID) const
{
    const SectionRange range = getSectionRange(sectionID);
    const floats accumLengths = _computeAccumulatedLengths(range);
    floats result;
    result.reserve(accumLengths.size());
    const float distance = getDistanceToSoma(sectionID);
    for (const float length : accumLengths)
        result.push_back(distance + length);

    return result;
}

const uint32_ts& Morphology::Impl::getChildren(const uint32_t sectionID) const
{
    return _sectionChildren[sectionID];
}

void Morphology::Impl::_transform(brion::MorphologyPtr morphology)
{
    auto& points = morphology->getPoints();
#pragma omp parallel for
    for (size_t i = 0; i < points.size(); ++i)
    {
        auto& p = points[i];
        const Vector3f& pp = transformation * p.get_sub_vector<3, 0>();
        p.set_sub_vector<3, 0>(pp);
    }
}

void Morphology::Impl::_extractInformation()
{
    // children list
    std::map<uint32_t, uint32_ts> children;
    const auto& sections = data->getSections();
    for (size_t i = 0; i < sections.size(); ++i)
    {
        const int32_t parent = sections[i][1];
        if (parent != -1)
            children[parent].push_back(i);
    }
    _sectionChildren.resize(sections.size());
    for (auto& sectionAndChildren : children)
    {
        _sectionChildren[sectionAndChildren.first].swap(
            sectionAndChildren.second);
    }

    // soma
    const uint32_ts ids = getSectionIDs({SectionType::soma}, false);
    if (ids.size() != 1)
        LBTHROW(std::runtime_error(
            "Bad input morphology '" +
            std::to_string(data->getInitData().getURI()) +
            "': " + std::to_string(ids.size()) + " somas found"));
    somaSection = ids[0];
}

float Morphology::Impl::_computeSectionLength(const uint32_t sectionID) const
{
    const auto& points = data->getPoints();
    const SectionRange range = getSectionRange(sectionID);
    float length = 0;
    for (size_t i = range.first; i != range.second - 1; ++i)
    {
        const Vector4f& start = points[i];
        const Vector4f& end = points[i + 1];
        const Vector3f& diff = (end - start).get_sub_vector<3, 0>();
        length += diff.length();
    }
    return length;
}

floats Morphology::Impl::_computeAccumulatedLengths(
    const SectionRange& range) const
{
    const auto& points = data->getPoints();
    floats result;
    result.reserve(range.second - range.first);
    result.push_back(0);
    for (size_t i = range.first; i != range.second - 1; ++i)
    {
        const Vector4f& start = points[i];
        const Vector4f& end = points[i + 1];
        const Vector3f& diff = (end - start).get_sub_vector<3, 0>();
        result.push_back(result.back() + diff.length());
    }
    return result;
}
}
}
