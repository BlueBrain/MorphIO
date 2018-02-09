#include <minimorph/properties.h>

namespace minimorph
{
namespace Property
{
template <> std::vector<Section::Type>& Properties::get<Section>() { return _sections;}
template <> const std::vector<Section::Type>& Properties::get<Section>() const { return _sections;}

template <> std::vector<Point::Type>& Properties::get<Point>() { return _points;}
template <> const std::vector<Point::Type>& Properties::get<Point>() const { return _points;}

template <> std::vector<SectionType::Type>& Properties::get<SectionType>() { return _sectionTypes;}
template <> const std::vector<SectionType::Type>& Properties::get<SectionType>() const { return _sectionTypes;}

template <> std::vector<Perimeter::Type>& Properties::get<Perimeter>() { return _perimeters;}
template <> const std::vector<Perimeter::Type>& Properties::get<Perimeter>() const { return _perimeters;}

template <> std::vector<Diameter::Type>& Properties::get<Diameter>() { return _diameters;}
template <> const std::vector<Diameter::Type>& Properties::get<Diameter>() const { return _diameters;}

}
}
