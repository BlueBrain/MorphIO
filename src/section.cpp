#include <algorithm>  // all_of

#include <morphio/morphology.h>
#include <morphio/section.h>
#include <morphio/tools.h>
#include <morphio/vector_types.h>

namespace morphio {

bool Section::isHeterogeneous(bool downstream) const {
    auto predicate = [&](const Section& s) { return type() != s.type(); };
    if (downstream) {
        return std::any_of(breadth_begin(), breadth_end(), predicate);
    }
    return std::any_of(upstream_begin(), upstream_end(), predicate);
}

SectionType Section::type() const {
    return properties_->get<Property::SectionType>()[id_];
}

depth_iterator Section::depth_begin() const {
    return depth_iterator(*this);
}

depth_iterator Section::depth_end() const {
    return depth_iterator();
}

breadth_iterator Section::breadth_begin() const {
    return breadth_iterator(*this);
}

breadth_iterator Section::breadth_end() const {
    return breadth_iterator();
}

upstream_iterator Section::upstream_begin() const {
    return upstream_iterator(*this);
}

upstream_iterator Section::upstream_end() const {
    return upstream_iterator();
}

range<const Point> Section::points() const {
    return get<Property::Point>();
}

range<const floatType> Section::diameters() const {
    return get<Property::Diameter>();
}

range<const floatType> Section::perimeters() const {
    return get<Property::Perimeter>();
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
