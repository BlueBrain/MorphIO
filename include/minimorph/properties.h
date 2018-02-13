#pragma once

#include <minimorph/types.h>

namespace minimorph
{

namespace Property
{
struct Section{
    typedef Vector2i Type;
};

struct Point{
    typedef minimorph::Point Type;
};

struct SectionType{
    typedef minimorph::SectionType Type;
};

struct Perimeter{
    typedef float Type;
};

struct Diameter {
    typedef float Type;
};

struct PointLevel {
    std::vector<Point::Type> _points;
    std::vector<Perimeter::Type> _perimeters;
    std::vector<Diameter::Type> _diameters;

    PointLevel() {}
    PointLevel(const PointLevel& data, SectionRange range);
};

struct SectionLevel {
    std::vector<Section::Type> _sections;
    std::vector<SectionType::Type> _sectionTypes;
    std::map<uint32_t, std::vector<uint32_t>> _children;
};

struct CellLevel {
    minimorph::CellFamily _cellFamily;
    MorphologyVersion _version;
};

struct Properties {
    PointLevel _pointLevel;
    SectionLevel _sectionLevel;
    CellLevel _cellLevel;

    template <typename T> std::vector<typename T::Type>& get();
    template <typename T> const std::vector<typename T::Type>& get() const;

    const minimorph::MorphologyVersion& version() { return _cellLevel._version; }
    const minimorph::CellFamily& cellFamily() { return _cellLevel._cellFamily; }
    const std::map<uint32_t, std::vector<uint32_t>>& children() { return _sectionLevel._children; }
};

}
}
