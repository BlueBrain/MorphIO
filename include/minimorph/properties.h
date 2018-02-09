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

struct Properties {
    std::vector<Section::Type> _sections;
    std::vector<Point::Type> _points;
    std::vector<SectionType::Type> _sectionTypes;
    std::vector<Perimeter::Type> _perimeters;
    std::vector<Diameter::Type> _diameters;

    template <typename T> std::vector<typename T::Type>& get();
    template <typename T> const std::vector<typename T::Type>& get() const;

    std::map<uint32_t, uint32_ts> _children;
    minimorph::CellFamily _cellFamily;

    const minimorph::CellFamily& getCellFamily() { return _cellFamily; }
    const std::map<uint32_t, uint32_ts>& getChildren() { return _children; }
};

}
}
