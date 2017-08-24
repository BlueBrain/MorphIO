
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

#ifndef BRAIN_NEURON_MORPHOLOGYIMPL
#define BRAIN_NEURON_MORPHOLOGYIMPL

#include "morphology.h"

#include <brion/morphology.h>
#include <lunchbox/lfVector.h>
#include <vmmlib/matrix.hpp> // member

namespace brain
{
namespace neuron
{
typedef std::pair<size_t, size_t> SectionRange;

class Morphology::Impl
{
public:
    const brion::ConstMorphologyPtr data;

    const Matrix4f transformation;

    uint32_t somaSection;

    explicit Impl(const URI& source);
    Impl(const URI& source, const Matrix4f& transform);
    explicit Impl(brion::ConstMorphologyPtr morphology);
    Impl(brion::MorphologyPtr morphology, const Matrix4f& transform);
    Impl(const void* data, const size_t size);

    SectionRange getSectionRange(const uint32_t sectionID) const;

    uint32_ts getSectionIDs(const SectionTypes& requestedTypes,
                            bool excludeSoma) const;

    float getSectionLength(const uint32_t sectionID) const;

    Vector4fs getSectionSamples(const uint32_t sectionID) const;

    Vector4fs getSectionSamples(const uint32_t sectionID,
                                const floats& samplePoints) const;

    float getDistanceToSoma(const uint32_t sectionID) const;

    floats getSampleDistancesToSoma(const uint32_t sectionID) const;

    const uint32_ts& getChildren(const uint32_t sectionID) const;

private:
    // Distances caches. These caches need to be thread-safe to follow the
    // recommendations for C++11 about mutable and const correctness.
    // (http://herbsutter.com/2013/05/24/gotw-6a-const-correctness-part-1-3/)
    typedef lunchbox::LFVector<float> LFFloats;
    mutable LFFloats _distancesToSoma;
    mutable LFFloats _sectionLengths;

    std::vector<uint32_ts> _sectionChildren;

    void _transform(brion::MorphologyPtr morphology);
    void _extractInformation();
    float _computeSectionLength(const uint32_t sectionID) const;
    floats _computeAccumulatedLengths(const SectionRange& range) const;
};
}
}
#endif
