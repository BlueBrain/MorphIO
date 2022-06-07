#include <algorithm>  // all_of

#include <morphio/morphology.h>
#include <morphio/section.h>
#include <morphio/tools.h>
#include <morphio/vector_types.h>

#include "point_utils.h"

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
