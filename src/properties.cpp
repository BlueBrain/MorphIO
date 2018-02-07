#include <properties.hpp>

namespace minimorph
{
template <> std::vector<typename SectionProperty::Type>& Properties::get<SectionProperty>() { return _sections;}
template <> const std::vector<typename SectionProperty::Type>& Properties::get<SectionProperty>() const { return _sections;}

template <> std::vector<typename PointProperty::Type>& Properties::get<PointProperty>() { return _points;}
template <> const std::vector<typename PointProperty::Type>& Properties::get<PointProperty>() const { return _points;}

template <> const std::vector<typename SectionTypeProperty::Type>& Properties::get<SectionTypeProperty>() const { return _sectionTypes;}
template <> std::vector<typename SectionTypeProperty::Type>& Properties::get<SectionTypeProperty>() { return _sectionTypes;}

template <> std::vector<typename PerimeterProperty::Type>& Properties::get<PerimeterProperty>() { return _perimeters;}
template <> const std::vector<typename PerimeterProperty::Type>& Properties::get<PerimeterProperty>() const { return _perimeters;}

template <> const std::vector<typename DiameterProperty::Type>& Properties::get<DiameterProperty>() const { return _diameters;}
template <> std::vector<typename DiameterProperty::Type>& Properties::get<DiameterProperty>() { return _diameters;}
}
