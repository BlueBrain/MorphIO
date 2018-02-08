#pragma once

#include <types.h>

namespace minimorph
{

namespace Property
{
struct Section{
    typedef std::vector<Vector2i> Type;
};

struct Point{
    typedef std::vector<minimorph::Point> Type;
};

struct SectionType{
    typedef std::vector<minimorph::SectionType> Type;
};

struct Perimeter{
    typedef std::vector<float> Type;
};

struct Diameter {
    typedef std::vector<float> Type;
};

struct Children {
    typedef std::map<uint32_t, uint32_ts> Type;
};

struct Properties {
    Point::Type _points;
    SectionType::Type _sectionTypes;
    Perimeter::Type _perimeters;
    Diameter::Type _diameters;
    Section::Type _sections;
    Children::Type _children;

    template <typename T> typename T::Type& get();
    template <typename T> const typename T::Type& get() const;
};

}
}
