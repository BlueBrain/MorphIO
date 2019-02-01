#include <morphio/morphology.h>
#include <morphio/section.h>
#include <morphio/vector_types.h>

namespace morphio {
SectionType Section::type() const
{
    auto val = _properties->get<Property::SectionType>()[_id];
    return val;
}

float Section::length() const
{
    auto points = this->points();
    int last = points.size() - 1;
    if (last < 1)
        return 0;

    return distance(points[0], points[last]);
}

depth_iterator Section::depth_begin() const
{
    return depth_iterator(*this);
}

depth_iterator Section::depth_end() const
{
    return depth_iterator();
}

breadth_iterator Section::breadth_begin() const
{
    return breadth_iterator(*this);
}

breadth_iterator Section::breadth_end() const
{
    return breadth_iterator();
}

upstream_iterator Section::upstream_begin() const
{
    return upstream_iterator(*this);
}

upstream_iterator Section::upstream_end() const
{
    return upstream_iterator();
}

const range<const Point> Section::points() const
{
    return get<Property::Point>();
}

const range<const float> Section::diameters() const
{
    return get<Property::Diameter>();
}

const range<const float> Section::perimeters() const
{
    return get<Property::Perimeter>();
}

} // namespace morphio

std::ostream& operator<<(std::ostream& os, const morphio::Section& section)
{
    os << "id: " << section.id() << std::endl;
    ;

    os << section.points();
    return os;
}

// operator<< must be defined in the global namespace to be usable there
std::ostream& operator<<(std::ostream& os,
    const morphio::range<const morphio::Point> points)
{
    for (auto point : points)
        os << point[0] << ' ' << point[1] << ' ' << point[2] << std::endl;
    return os;
}
