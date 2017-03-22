
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

#include <lunchbox/log.h>

#include <boost/foreach.hpp>

#include <bitset>

namespace brain
{
namespace neuron
{
namespace
{
template <typename T>
void _serializeArray(unsigned char*& dst,
                     const std::shared_ptr<std::vector<T>>& src)
{
    const size_t arraySize = src->size();
    *reinterpret_cast<size_t*>(dst) = arraySize;
    dst += sizeof(size_t);
    memcpy(dst, src->data(), sizeof(T) * src->size());
    dst += sizeof(T) * src->size();
}

template <typename T>
void _deserializeArray(std::shared_ptr<std::vector<T>>& dst,
                       const unsigned char*& src)
{
    const size_t arraySize = *reinterpret_cast<const size_t*>(src);
    src += sizeof(size_t);
    const T* dstPtr = reinterpret_cast<const T*>(src);
    dst.reset(new std::vector<T>(dstPtr, dstPtr + arraySize));
    src += sizeof(T) * arraySize;
}
}

Morphology::Impl::Impl(const void* data, const size_t size)
{
    _fromBinary(data, size);
}

Morphology::Impl::Impl(const brion::Morphology& morphology)
    : points(morphology.readPoints(brion::enums::MORPHOLOGY_UNDEFINED))
    , sections(morphology.readSections(brion::enums::MORPHOLOGY_UNDEFINED))
    , types(morphology.readSectionTypes())
    , apicals(morphology.readApicals())
{
    _extractChildrenLists();

    const uint32_ts ids = getSectionIDs({SectionType::soma}, false);
    if (ids.size() != 1)
        LBTHROW(std::runtime_error(
            "Bad input morphology. None or more than one soma found"));
    somaSection = ids[0];
}

bool Morphology::Impl::_fromBinary(const void* data, const size_t size)
{
    const unsigned char* ptr = reinterpret_cast<const unsigned char*>(data);

    _deserializeArray(points, ptr);
    _deserializeArray(sections, ptr);
    _deserializeArray(types, ptr);
    if (size_t(ptr - reinterpret_cast<const unsigned char*>(data)) < size)
        _deserializeArray(apicals, ptr);

    _extractChildrenLists();

    const uint32_ts ids = getSectionIDs({SectionType::soma}, false);

    if (ids.size() != 1)
        LBTHROW(std::runtime_error(
            "Bad input morphology. None or more than one soma found"));
    somaSection = ids[0];
    return true;
}

servus::Serializable::Data Morphology::Impl::_toBinary() const
{
    servus::Serializable::Data data;

    data.size = sizeof(size_t) + sizeof(brion::Vector4f) * points->size() +
                sizeof(size_t) + sizeof(brion::Vector2i) * sections->size() +
                sizeof(size_t) + sizeof(uint32_t) * types->size();
    if (!apicals->empty())
        data.size += sizeof(size_t) + sizeof(brion::Vector2i) * apicals->size();

    unsigned char* ptr = new unsigned char[data.size];
    data.ptr.reset(ptr);

    _serializeArray(ptr, points);
    _serializeArray(ptr, sections);
    _serializeArray(ptr, types);
    if (!apicals->empty())
        _serializeArray(ptr, apicals);

    return data;
}

SectionRange Morphology::Impl::getSectionRange(const uint32_t sectionID) const
{
    const size_t start = (*sections)[sectionID][0];
    const size_t end = sectionID == sections->size() - 1
                           ? points->size()
                           : (*sections)[sectionID + 1][0];
    return std::make_pair(start, end);
}

uint32_ts Morphology::Impl::getSectionIDs(const SectionTypes& requestedTypes,
                                          const bool excludeSoma) const
{
    std::bitset<size_t(SectionType::apicalDendrite)> bits;
    BOOST_FOREACH (const SectionType type, requestedTypes)
    {
        if (type != SectionType::soma || !excludeSoma)
            bits[size_t(type)] = true;
    }

    uint32_ts result;
    for (size_t i = 0; i != types->size(); ++i)
    {
        const SectionType type = static_cast<SectionType>((*types)[i]);
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

    if (length == 0 && (*types)[sectionID] != brion::enums::SECTION_SOMA)
        length = _computeSectionLength(sectionID);
    return length;
}

Vector4fs Morphology::Impl::getSectionSamples(const uint32_t sectionID) const
{
    const SectionRange range = getSectionRange(sectionID);
    Vector4fs result;
    result.reserve(range.second - range.first);
    result.insert(result.end(), points->begin() + range.first,
                  points->begin() + range.second);
    return result;
}

Vector4fs Morphology::Impl::getSectionSamples(const uint32_t sectionID,
                                              const floats& samplePoints) const
{
    const SectionRange range = getSectionRange(sectionID);

    // If the section is the soma return directly the soma position.
    if ((*types)[sectionID] == brion::enums::SECTION_SOMA)
        // This code shouldn't be reached.
        LBTHROW(std::runtime_error("Invalid method called on soma section"));

    // Dealing with the degenerate case of single point sections.
    if (range.first + 1 == range.second)
        return Vector4fs(samplePoints.size(), (*points)[range.first]);

    Vector4fs result;
    result.reserve(samplePoints.size());

    const floats accumLengths = _computeAccumulatedLengths(range);
    const float totalLength = accumLengths.back();

    BOOST_FOREACH (const float point, samplePoints)
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
            result.push_back((*points)[start]);
            continue;
        }

        // Interpolating the cross section at point.
        const float alpha = (length - accumLengths[index]) /
                            (accumLengths[index + 1] - accumLengths[index]);
        const Vector4f sample =
            (*points)[start + 1] * alpha + (*points)[start] * (1 - alpha);
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
        const int32_t parent = (*sections)[sectionID][1];
        if (parent == -1 || (*types)[parent] == brion::enums::SECTION_SOMA)
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
    BOOST_FOREACH (const float length, accumLengths)
        result.push_back(distance + length);

    return result;
}

const uint32_ts& Morphology::Impl::getChildren(const uint32_t sectionID) const
{
    return _sectionChildren[sectionID];
}

void Morphology::Impl::transform(const Matrix4f& matrix)
{
#pragma omp parallel for
    for (size_t i = 0; i < points->size(); ++i)
    {
        Vector4f& p = (*points)[i];
        const Vector3f& pp = matrix * p.get_sub_vector<3, 0>();
        p.set_sub_vector<3, 0>(pp);
    }
}

void Morphology::Impl::_extractChildrenLists()
{
    typedef std::map<uint32_t, uint32_ts> ChildrenMap;
    ChildrenMap children;
    for (size_t i = 0; i < sections->size(); ++i)
    {
        const int32_t parent = (*sections)[i][1];
        if (parent != -1)
            children[parent].push_back(i);
    }
    _sectionChildren.resize(sections->size());
    BOOST_FOREACH (ChildrenMap::value_type& sectionAndChildren, children)
    {
        _sectionChildren[sectionAndChildren.first].swap(
            sectionAndChildren.second);
    }
}

float Morphology::Impl::_computeSectionLength(const uint32_t sectionID) const
{
    const SectionRange range = getSectionRange(sectionID);
    float length = 0;
    for (size_t i = range.first; i != range.second - 1; ++i)
    {
        const Vector4f& start = (*points)[i];
        const Vector4f& end = (*points)[i + 1];
        const Vector3f& diff = (end - start).get_sub_vector<3, 0>();
        length += diff.length();
    }
    return length;
}

floats Morphology::Impl::_computeAccumulatedLengths(
    const SectionRange& range) const
{
    floats result;
    result.reserve(range.second - range.first);
    result.push_back(0);
    for (size_t i = range.first; i != range.second - 1; ++i)
    {
        const Vector4f& start = (*points)[i];
        const Vector4f& end = (*points)[i + 1];
        const Vector3f& diff = (end - start).get_sub_vector<3, 0>();
        result.push_back(result.back() + diff.length());
    }
    return result;
}
}
}
