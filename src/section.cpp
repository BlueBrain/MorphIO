#include <morphio/morphology.h>
#include <morphio/section.h>
#include <morphio/vector_types.h>

namespace morphio {

bool Section::diff(const Section& other, bool verbose) const
{
    if (this->type() != other.type()) {
        if (verbose)
            std::cout << "Reason: section type differ" << std::endl;
        return true;
    }

    if (this->points() != other.points()) {
        if (verbose)
            std::cout << "Reason: points differ" << std::endl;
        return true;
    }

    if (this->diameters() != other.diameters()) {
        if (verbose)
            std::cout << "Reason: diameters differ" << std::endl;
        return true;
    }

    if (this->perimeters() != other.perimeters()) {
        if (verbose)
            std::cout << "Reason: perimeters differ" << std::endl;
        return true;
    }

    if (this->children().size() != other.children().size()) {
        if (verbose)
            std::cout << "Reason: different number of children" << std::endl;
        return true;
    }

    for (unsigned int i = 0; i < this->children().size(); ++i)
        if (this->children()[i].diff(other.children()[i], verbose)) {
            if (verbose)
            {
                std::cout << "Summary: children of ";
                ::operator<<(std::cout, *this);
                std::cout << " differ. See the above \"Reason\" to know in what they differ." << std::endl;
            }

            return true;
        }

    return false;
}

bool Section::operator==(const Section& other) const
{
    return !diff(other, false);
}

bool Section::operator!=(const Section& other) const
{
    return diff(other, false);
}

SectionType Section::type() const
{
    auto val = _properties->get<Property::SectionType>()[_id];
    return val;
}

float Section::length() const
{
    auto points_ = this->points();
    if (points_.size() < 2)
        return 0;

    size_t last = points_.size() - 1;
    return distance(points_[0], points_[last]);
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
    auto points = section.points();
    os << "Section(id=" << section.id() << ", points=[(" << points[0] << "),..., (";
    os << points[points.size() - 1] << ")])";
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
