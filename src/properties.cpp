#include <algorithm>

#include <morphio/properties.h>

bool verbose = true;

namespace std
{
template <typename T, size_t N> string to_string(const array<T, N>& a){
    string res("[");
    for(auto el: a)
        res += to_string(el) + ", ";
    return res;
}
}

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



template <typename T> bool compare(const std::vector<T>& vec1, const std::vector<T>& vec2, const std::string& name, bool verbose) {
    if(vec1 == vec2)
        return true;
    if(vec1.size() != vec2.size()){
        LBERROR("Error comparing "+name+", size differs: " + std::to_string(vec1.size()) + " vs " + std::to_string(vec2.size()));
        return false;
    }
    for(int i=0;i<vec1.size();++i){
        LBERROR(std::to_string(vec1[i]) + " <--> " + std::to_string(vec2[i]));
    }
    return false;
}

template <typename T, typename U> bool compare(const std::map<T,U>& vec1, const std::map<T,U>& vec2, const std::string& name, bool verbose) {
    if(vec1 == vec2)
        return true;
    if(vec1.size() != vec2.size()){
        LBERROR("Error comparing "+name+", size differs: " + std::to_string(vec1.size()) + " vs " + std::to_string(vec2.size()));
    }
    return false;
}

template <typename T> bool compare(const T& el1, const T& el2, const std::string& name, bool verbose) {
    if(el1 == el2)
        return true;
    LBERROR(name + " differs") ;
    return false;
}

bool PointLevel::operator==(const PointLevel& other) const {
    return this == &other ||
        (compare(this->_points, other._points, "_points", verbose) &&
         compare(this->_diameters, other._diameters, "_diameters", verbose) &&
         compare(this->_perimeters, other._perimeters, "_perimeters", verbose));
}

bool PointLevel::operator!=(const PointLevel& other) const {
    return !(this->operator==(other));
}


bool SectionLevel::operator==(const SectionLevel& other) const {
    return this == &other ||
        (compare(this->_sections, other._sections, "_sections", verbose) &&
         compare(this->_sectionTypes, other._sectionTypes, "_sectionTypes", verbose) &&
         compare(this->_children, other._children, "_childre", verbose));
}

bool SectionLevel::operator!=(const SectionLevel& other) const {
    return !(this->operator==(other));
}

bool CellLevel::operator==(const CellLevel& other) const {
    if(verbose && this->_cellFamily != other._cellFamily){
        std::cout << "this->_cellFamily: " << this->_cellFamily << std::endl;
        std::cout << "other._cellFamily: " << other._cellFamily << std::endl;
    }
    return this == &other ||
        (this->_cellFamily == other._cellFamily
         // this->_somaType == other._somaType
            );
}

bool CellLevel::operator!=(const CellLevel& other) const {
    return !(this->operator==(other));
}


bool Properties::operator==(const Properties& other) const {
    return this == &other ||
        (compare(this -> _pointLevel, other._pointLevel, "_pointLevel", verbose) &&
         compare(this -> _sectionLevel, other._sectionLevel, "_sectionLevel", verbose) &&
         compare(this -> _cellLevel, other._cellLevel, "_cellLevel", verbose));
}

bool Properties::operator!=(const Properties& other) const {
    return !this->operator==(other);
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
