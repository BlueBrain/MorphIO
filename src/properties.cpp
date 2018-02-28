#include <algorithm>

#include <morphio/properties.h>

namespace morphio
{
namespace Property
{
template <typename T>
std::vector<typename T::Type> copySpan(
    const std::vector<typename T::Type>& data, SectionRange range)
{
    if (data.empty())
        return std::vector<typename T::Type>();
    return std::vector<typename T::Type>(data.begin() + range.first,
                                         data.begin() + range.second);
}

PointLevel::PointLevel(const PointLevel &data) :
    _points(data._points),
    _diameters(data._diameters),
    _perimeters(data._perimeters)
{
}

PointLevel::PointLevel(const PointLevel& data, SectionRange range)
{
    _points = copySpan<Property::Point>(data._points, range);
    _diameters = copySpan<Property::Diameter>(data._diameters, range);
    _perimeters = copySpan<Property::Perimeter>(data._perimeters, range);
}

template <>
std::vector<Section::Type>& Properties::get<Section>()
{
    return _sectionLevel._sections;
}
template <>
const std::vector<Section::Type>& Properties::get<Section>() const
{
    return _sectionLevel._sections;
}

template <>
std::vector<Point::Type>& Properties::get<Point>()
{
    return _pointLevel._points;
}
template <>
const std::vector<Point::Type>& Properties::get<Point>() const
{
    return _pointLevel._points;
}

template <>
std::vector<SectionType::Type>& Properties::get<SectionType>()
{
    return _sectionLevel._sectionTypes;
}
template <>
const std::vector<SectionType::Type>& Properties::get<SectionType>() const
{
    return _sectionLevel._sectionTypes;
}

template <>
std::vector<Perimeter::Type>& Properties::get<Perimeter>()
{
    return _pointLevel._perimeters;
}
template <>
const std::vector<Perimeter::Type>& Properties::get<Perimeter>() const
{
    return _pointLevel._perimeters;
}

template <>
std::vector<Diameter::Type>& Properties::get<Diameter>()
{
    return _pointLevel._diameters;
}
template <>
const std::vector<Diameter::Type>& Properties::get<Diameter>() const
{
    return _pointLevel._diameters;
}


std::ostream& operator<<(std::ostream& os, const PointLevel& prop){
    os << "Point level properties:" << std::endl;
    os << "Point Diameter" << (prop._perimeters.size() == prop._points.size() ? " Perimeter" : "") << std::endl;
    for(int i = 0; i<prop._points.size(); ++i){
        os << dumpPoint(prop._points[i]) << ' ' << prop._diameters[i];
        if(prop._perimeters.size() == prop._points.size())
            os << ' ' << prop._perimeters[i];
        os << std::endl;
    }
    return os;
}

std::ostream& operator<<(std::ostream& os, const Properties& properties){
    os << properties._pointLevel << std::endl;
    // os << _sectionLevel << std::endl;
    // os << _cellLevel << std::endl;
    return os;
}

} // namespace Property
} // namespace morphio
