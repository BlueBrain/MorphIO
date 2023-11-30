/* Copyright (c) 2013-2023, EPFL/Blue Brain Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <algorithm>  // any_of

#include <morphio/section.h>

#include "point_utils.h"  // operator<<

namespace morphio {

bool Section::isHeterogeneous(bool downstream) const {
    auto predicate = [&](const Section& s) { return type() != s.type(); };
    if (downstream) {
        return std::any_of(breadth_begin(), breadth_end(), predicate);
    }
    return std::any_of(upstream_begin(), upstream_end(), predicate);
}

bool Section::hasSameShape(const Section& other) const noexcept {
    return (other.type() == type() && other.diameters() == diameters() &&
            other.points() == points() && other.perimeters() == perimeters());
}

}  // namespace morphio

std::ostream& operator<<(std::ostream& os, const morphio::Section& section) {
    const auto& points = section.points();
    if (points.empty()) {
        os << "Section(id=" << section.id() << ", points=[])";
    } else {
        os << "Section(id=" << section.id() << ", points=[(" << points[0] << "),..., ("
           << points[points.size() - 1] << ")])";
    }
    return os;
}
