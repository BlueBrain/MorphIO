#pragma once

#include <morphio/types.h>

namespace morphio {
namespace vasculature {
namespace property {
struct VascSection
{
    // offset
    // refers to the index in the points vector from which the section begins
    using Type = unsigned int;
};

struct Point
{
    using Type = morphio::Point;
};

struct SectionType
{
    using Type = morphio::VascularSectionType;
};

struct Diameter
{
    using Type = float;
};

struct Connection
{
    // stores the graph connectivity between the sections
    // If section1 is connected to section2, then the last point of section1
    // and the first point of section2 must be equal.
    using Type = std::array<unsigned int, 2>;
};

struct VascPointLevel
{
    // stores point level information
    std::vector<Point::Type> _points;
    std::vector<Diameter::Type> _diameters;

    VascPointLevel() {}
    VascPointLevel(std::vector<Point::Type> points,
        std::vector<Diameter::Type> diameters);
    VascPointLevel(const VascPointLevel& data);
    VascPointLevel(const VascPointLevel& data, SectionRange range);
    VascPointLevel& operator=(const VascPointLevel&) = default;
};

struct VascEdgeLevel
{
    // stores edge level information, more attributes can be added later
    std::vector<float> leakiness;
};

struct VascSectionLevel
{
    // stores section level information
    std::vector<VascSection::Type> _sections;
    std::vector<SectionType::Type> _sectionTypes;
    std::map<uint32_t, std::vector<uint32_t>> _predecessors;
    std::map<uint32_t, std::vector<uint32_t>> _successors;
    bool operator==(const VascSectionLevel& other) const;
    bool operator!=(const VascSectionLevel& other) const;
};

struct Properties
{
    VascPointLevel _pointLevel;
    VascEdgeLevel _edgeLevel;
    VascSectionLevel _sectionLevel;
    std::vector<Connection::Type> _connectivity;

    template <typename T>
    std::vector<typename T::Type>& get();
    template <typename T>
    const std::vector<typename T::Type>& get() const;

    const std::map<uint32_t, std::vector<uint32_t>>& predecessors();
    const std::map<uint32_t, std::vector<uint32_t>>& successors();

    bool operator==(const Properties& other) const;
    bool operator!=(const Properties& other) const;
};


std::ostream& operator<<(std::ostream& os, const Properties& properties);
std::ostream& operator<<(std::ostream& os, const VascPointLevel& pointLevel);

template <>
std::vector<Point::Type>& Properties::get<Point>();
template <>
std::vector<Diameter::Type>& Properties::get<Diameter>();
template <>
std::vector<SectionType::Type>& Properties::get<SectionType>();
template <>
std::vector<VascSection::Type>& Properties::get<VascSection>();
template <>
const std::vector<VascSection::Type>& Properties::get<VascSection>() const;
template <>
std::vector<Connection::Type>& Properties::get<Connection>();
}
}
}

namespace std
{
extern template string to_string<float, 3>(const array<float, 3>&);
}
