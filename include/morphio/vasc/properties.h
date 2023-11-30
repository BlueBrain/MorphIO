/* Copyright (c) 2013-2023, EPFL/Blue Brain Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include <map>
#include <string>  // std::string
#include <vector>  // std::vector

#include <morphio/types.h>

namespace morphio {
namespace vasculature {
namespace property {

struct VascSection {
    // offset
    // refers to the index in the points vector from which the section begins
    using Type = unsigned int;
};

struct Point {
    using Type = morphio::Point;
};

struct SectionType {
    using Type = morphio::VascularSectionType;
};

struct Diameter {
    using Type = floatType;
};

/**
   Stores the graph connectivity between the sections. If section1 is connected to section2,
   then the last point of section1 and the first point of section2 must be equal.
 */
struct Connection {
    using Type = std::array<unsigned int, 2>;
};

/** Stores point level information. Similar to morphio::PointLevel */
struct VascPointLevel {
    std::vector<Point::Type> _points;
    std::vector<Diameter::Type> _diameters;

    VascPointLevel() = default;
    VascPointLevel(const std::vector<Point::Type>& points,
                   const std::vector<Diameter::Type>& diameters);
    VascPointLevel(const VascPointLevel& data);
    VascPointLevel(const VascPointLevel& data, SectionRange range);
    VascPointLevel& operator=(const VascPointLevel&) = default;
};

/** Stores edge level information */
struct VascEdgeLevel {
    std::vector<morphio::floatType> leakiness;
};

/** stores section level information */
struct VascSectionLevel {
    std::vector<VascSection::Type> _sections;
    std::vector<SectionType::Type> _sectionTypes;
    std::map<uint32_t, std::vector<uint32_t>> _predecessors;
    std::map<uint32_t, std::vector<uint32_t>> _successors;
    bool operator==(const VascSectionLevel& other) const;
    bool operator!=(const VascSectionLevel& other) const;

    // Like operator!= but with logLevel argument
    bool diff(const VascSectionLevel& other, LogLevel logLevel) const;
};

/** Class that holds all other levels(point, edge, etc.) information */
struct Properties {
    VascPointLevel _pointLevel;
    VascEdgeLevel _edgeLevel;
    VascSectionLevel _sectionLevel;
    std::vector<Connection::Type> _connectivity;

    template <typename T>
    std::vector<typename T::Type>& get_mut() noexcept;

    template <typename T>
    const std::vector<typename T::Type>& get() const noexcept;

    const std::map<uint32_t, std::vector<uint32_t>>& predecessors() const noexcept {
        return _sectionLevel._predecessors;
    }
    const std::map<uint32_t, std::vector<uint32_t>>& successors() const noexcept {
        return _sectionLevel._successors;
    }
};

std::ostream& operator<<(std::ostream& os, const Properties& properties);
std::ostream& operator<<(std::ostream& os, const VascPointLevel& pointLevel);

#define INSTANTIATE_TEMPLATE_GET(T, M)                                       \
    template <>                                                              \
    inline std::vector<T::Type>& Properties::get_mut<T>() noexcept {         \
        return M;                                                            \
    }                                                                        \
    template <>                                                              \
    inline const std::vector<T::Type>& Properties::get<T>() const noexcept { \
        return M;                                                            \
    }

INSTANTIATE_TEMPLATE_GET(VascSection, _sectionLevel._sections)
INSTANTIATE_TEMPLATE_GET(Point, _pointLevel._points)
INSTANTIATE_TEMPLATE_GET(Connection, _connectivity)
INSTANTIATE_TEMPLATE_GET(SectionType, _sectionLevel._sectionTypes)
INSTANTIATE_TEMPLATE_GET(Diameter, _pointLevel._diameters)

#undef INSTANTIATE_TEMPLATE_GET


}  // namespace property
}  // namespace vasculature
}  // namespace morphio
