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


;

PointLevel::PointLevel(std::vector<Point::Type> points,
                       std::vector<Diameter::Type> diameters,
                       std::vector<Perimeter::Type> perimeters)
 : _points(points), _diameters(diameters), _perimeters(perimeters)
{
    if(_points.size() != _diameters.size())
        throw SectionBuilderError("Point vector have size: " + std::to_string(_points.size())
                                  + " while Diameter vector has size: " + std::to_string(_diameters.size()));

    if(_perimeters.size() > 0 && _points.size() != _perimeters.size())
        throw SectionBuilderError("Point vector have size: " + std::to_string(_points.size())
                                  + " while Perimeter vector has size: " + std::to_string(_perimeters.size()));

}


PointLevel::PointLevel(const PointLevel &data) :
    PointLevel(data._points, data._diameters, data._perimeters)
{
}

PointLevel::PointLevel(const PointLevel& data, SectionRange range)
{
    _points = copySpan<Property::Point>(data._points, range);
    _diameters = copySpan<Property::Diameter>(data._diameters, range);
    _perimeters = copySpan<Property::Perimeter>(data._perimeters, range);
}

bool PointLevel::operator==(const PointLevel& other) const {
    std::cout << "this->_points == other._points: " << (this->_points == other._points) << std::endl;
    // std::cout << "dumpPoints(this->_points): " << dumpPoints(this->_points) << std::endl;
    // std::cout << "dumpPoints(other._points): " << dumpPoints(other._points) << std::endl;
    std::cout << "this->points.size(): " << this->_points.size() << std::endl;
    std::cout << "other._points.size(): " << other._points.size() << std::endl;
    for(int i=0;i<this->_points.size();++i){
        if(this->_points[i] != other._points[i]) {
            std::cout << dumpPoint(this->_points[i]) << " -> " << dumpPoint(other._points[i]) << std::endl;
            return false;

        }


    }
    return this == &other ||
        (this->_points == other._points &&
         this->_perimeters == other._perimeters &&
         this->_diameters == other._diameters);
}

bool SectionLevel::operator==(const SectionLevel& other) const {
    if(this->_sectionTypes != other._sectionTypes) {
        std::cout << "this->_sectionTypes.size() : " << this->_sectionTypes.size()  << std::endl;
        std::cout << "other._sectionTypes.size(): " << other._sectionTypes.size() << std::endl;
    }
    return this == &other ||
        (this->_sections == other._sections &&
         this->_sectionTypes == other._sectionTypes &&
         this->_children == other._children);
}

bool CellLevel::operator==(const CellLevel& other) const {
    if(this->_cellFamily != other._cellFamily){
        std::cout << "this->_cellFamily: " << this->_cellFamily << std::endl;
        std::cout << "other._cellFamily: " << other._cellFamily << std::endl;
    }
    // if(this->_somaType != other._somaType) {
    //     std::cout << "this->somaType: " << this->_somaType << std::endl;
    //     std::cout << "other._somaType: " << other._somaType << std::endl;
    // }
    return this == &other ||
        (this->_cellFamily == other._cellFamily
         // this->_somaType == other._somaType
            );
}

bool Properties::operator==(const Properties& other) const {
    std::cout << "this -> _pointLevel == other._pointLevel: " << (this -> _pointLevel == other._pointLevel) << std::endl;
    std::cout << "this -> _sectionLevel == other._sectionLevel: " << (this -> _sectionLevel == other._sectionLevel) << std::endl;
    std::cout << "this -> _cellLevel == other._cellLevel: " << (this -> _cellLevel == other._cellLevel) << std::endl;
    return this == &other ||
        (this -> _pointLevel == other._pointLevel &&
         this -> _sectionLevel == other._sectionLevel &&
         this -> _cellLevel == other._cellLevel);
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
