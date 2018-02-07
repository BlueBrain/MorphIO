#pragma once

#include <types.h>

namespace minimorph
{

struct SectionProperty {
    typedef Vector2i Type;
};

struct PointProperty {
    typedef Point Type;
};

struct SectionTypeProperty {
    typedef SectionType Type;
};

struct PerimeterProperty {
    typedef float Type;
};

struct DiameterProperty {
    typedef float Type;
};

struct Properties {
    std::vector<PointProperty::Type> _points;
    std::vector<SectionTypeProperty::Type> _sectionTypes;
    std::vector<PerimeterProperty::Type> _perimeters;
    std::vector<DiameterProperty::Type> _diameters;
    std::vector<SectionProperty::Type> _sections;

    template <typename T> std::vector<typename T::Type>& get();
    template <typename T> const std::vector<typename T::Type>& get() const ;


};


}
