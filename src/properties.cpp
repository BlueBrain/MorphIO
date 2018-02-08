#include <properties.hpp>

namespace minimorph
{
namespace Property
{
template <> Section::Type& Properties::get<Section>() { return _sections;}
template <> const Section::Type& Properties::get<Section>() const { return _sections;}

template <> Point::Type& Properties::get<Point>() { return _points;}
template <> const Point::Type& Properties::get<Point>() const { return _points;}

template <> const SectionType::Type& Properties::get<SectionType>() const { return _sectionTypes;}
template <> SectionType::Type& Properties::get<SectionType>() { return _sectionTypes;}

template <> Perimeter::Type& Properties::get<Perimeter>() { return _perimeters;}
template <> const Perimeter::Type& Properties::get<Perimeter>() const { return _perimeters;}

template <> const Diameter::Type& Properties::get<Diameter>() const { return _diameters;}
template <> Diameter::Type& Properties::get<Diameter>() { return _diameters;}

template <> const Children::Type& Properties::get<Children>() const { return _children;}
template <> Children::Type& Properties::get<Children>() { return _children;}

template <> const CellFamily::Type& Properties::get<CellFamily>() const { return _cellFamily;}
template <> CellFamily::Type& Properties::get<CellFamily>() { return _cellFamily;}

// template <> const Soma::Type& Properties::get<Soma>() const { return _soma;}
// template <> Soma::Type& Properties::get<Soma>() { return _soma;}
}
}
