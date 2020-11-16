#include <morphio/morphology.h>
#include <morphio/section.h>
#include <morphio/tools.h>
#include <morphio/vector_types.h>

namespace morphio {

SectionType Section::type() const {
    auto val = _properties->get<Property::SectionType>()[_id];
    return val;
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

}  // namespace morphio

std::ostream& operator<<(std::ostream& os, const morphio::Section& section) {
    const auto& points = section.points();
    if (points.empty()) {
        os << "Section(id=" << section.id() << ", points=[])";
    } else {
        os << "Section(id=" << section.id() << ", points=[(" << points[0] << "),..., (";
        os << points[points.size() - 1] << ")])";
    }
    return os;
}

// operator<< must be defined in the global namespace to be usable there
std::ostream& operator<<(std::ostream& os, const morphio::range<const morphio::Point>& points) {
    for (const auto& point : points) {
        os << point[0] << ' ' << point[1] << ' ' << point[2] << '\n';
    }
    return os;
}
