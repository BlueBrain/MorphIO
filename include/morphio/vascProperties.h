#pragma once

#include <morphio/types.h>

namespace morphio
{
namespace VasculatureProperty
{

struct Section
{
    typedef std::array<int, 2> Type;
};

struct Point
{
    typedef morphio::Point Type;
};

struct SectionType
{
    typedef morphio::SectionType Type;
}

struct Diameter
{
    typedef float Type;
}

struct PointLevel
{
    std::vector<Point::Type> _points;
    std::vector<Diameter::Type> _diameters;

    PointLevel() {}
    PointLevel(std::vector<Point::Type> points,
               std::vector<Diameter::Type> diameters);
    PointLevel(const PointLevel& data);
    PointLevel(const PointLevel& data, SectionRange range);
};

struct EdgeLevel
{
    std::vector<float> leakiness;
}

struct SectionLevel
{
    std::vector<Section::Type> _sections;
    std::vector<SectionType::Type> _sectionTypes;
    std::map<int32_t, std::vector<uint32_t>> _predecessors;
    std::map<int32_t, std::vector<uint32_t>> _successors;

    bool operator==(const SectionLevel& other) const;
    bool operator!=(const SectionLevel& other) const;
};

struct Properties
{
    PointLevel _pointLevel;
    EdgeLevel _edgeLevel;
    SectionLevel _sectionLevel;

    template <typename T>
    std::vector<typename T::Type>& get();
    template <typename T>
    const std::vector<typename T::Type>& get() const;

    const std::map<int32_t, std::vector<uint32_t>>& neighbors();
};

std::ostream& operator<<(std::ostream& os, const Properties& properties);
std::ostream& operator<<(std::ostream& os, const PointLevel& pointLevel);

}
}
