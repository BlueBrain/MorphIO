/* Copyright (c) 2013-2023, EPFL/Blue Brain Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <morphio/mito_section.h>

namespace morphio {

mito_depth_iterator MitoSection::depth_begin() const {
    return mito_depth_iterator(*this);
}

mito_depth_iterator MitoSection::depth_end() const {
    return mito_depth_iterator();
}

mito_breadth_iterator MitoSection::breadth_begin() const {
    return mito_breadth_iterator(*this);
}

mito_breadth_iterator MitoSection::breadth_end() const {
    return mito_breadth_iterator();
}

mito_upstream_iterator MitoSection::upstream_begin() const {
    return mito_upstream_iterator(*this);
}

mito_upstream_iterator MitoSection::upstream_end() const {
    return mito_upstream_iterator();
}

range<const uint32_t> MitoSection::neuriteSectionIds() const {
    return get<Property::MitoNeuriteSectionId>();
}

range<const floatType> MitoSection::diameters() const {
    return get<Property::MitoDiameter>();
}

range<const floatType> MitoSection::relativePathLengths() const {
    return get<Property::MitoPathLength>();
}

bool MitoSection::hasSameShape(const MitoSection& other) const noexcept {
    return (other.neuriteSectionIds() == neuriteSectionIds() && other.diameters() == diameters() &&
            other.relativePathLengths() == relativePathLengths());
}
}  // namespace morphio
