/* Copyright (c) 2013-2023, EPFL/Blue Brain Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <morphio/errorMessages.h>
#include <morphio/properties.h>
#include <morphio/vasc/properties.h>
#include <string>

#include "../point_utils.h"
#include "../shared_utils.hpp"

namespace {
bool compare_section_structure(
    const std::vector<morphio::vasculature::property::VascSection::Type>& vec1,
    const std::vector<morphio::vasculature::property::VascSection::Type>& vec2,
    const std::string& name,
    morphio::LogLevel logLevel) {
    if (vec1.size() != vec2.size()) {
        if (logLevel > morphio::LogLevel::ERROR) {
            morphio::printError(morphio::Warning::UNDEFINED,
                                "Error comparing " + name +
                                    ", size differs: " + std::to_string(vec1.size()) + " vs " +
                                    std::to_string(vec2.size()));
        }
        return false;
    }

    for (size_t i = 1; i < vec1.size(); ++i) {
        if (vec1[i] - vec1[1] != vec2[i] - vec2[1]) {
            if (logLevel > morphio::LogLevel::ERROR) {
                morphio::printError(morphio::Warning::UNDEFINED,
                                    "Error comparing " + name + ", elements differ:");
                morphio::printError(morphio::Warning::UNDEFINED,
                                    std::to_string(vec1[i] - vec1[1]) + " <--> " +
                                        std::to_string(vec2[i] - vec2[1]));
            }
            return false;
        }
    }
    return true;
}


}  // namespace


namespace morphio {
namespace vasculature {
namespace property {

VascPointLevel::VascPointLevel(const std::vector<Point::Type>& points,
                               const std::vector<Diameter::Type>& diameters)
    : _points(points)
    , _diameters(diameters) {
    if (_points.size() != _diameters.size()) {
        throw SectionBuilderError(
            "Point vector have size: " + std::to_string(_points.size()) +
            "while Diameter vector has size: " + std::to_string(_diameters.size()));
    }
}

VascPointLevel::VascPointLevel(const VascPointLevel& data)
    : VascPointLevel(data._points, data._diameters) {}

VascPointLevel::VascPointLevel(const VascPointLevel& data, SectionRange range) {
    _points = copySpan<property::Point>(data._points, range);
    _diameters = copySpan<property::Diameter>(data._diameters, range);
}

bool VascSectionLevel::diff(const VascSectionLevel& other, LogLevel logLevel) const {
    return this == &other ||
           (compare_section_structure(this->_sections, other._sections, "_sections", logLevel) &&
            morphio::property::compare(
                this->_sectionTypes, other._sectionTypes, "_sectionTypes", logLevel) &&
            morphio::property::compare(
                this->_predecessors, other._predecessors, "_predecessors", logLevel) &&
            morphio::property::compare(
                this->_successors, other._successors, "_successors", logLevel));
}

bool VascSectionLevel::operator==(const VascSectionLevel& other) const {
    return !diff(other, LogLevel::ERROR);
}

bool VascSectionLevel::operator!=(const VascSectionLevel& other) const {
    return diff(other, LogLevel::ERROR);
}

std::ostream& operator<<(std::ostream& os, const VascPointLevel& pointLevel) {
    os << "Point level properties:\n";
    os << "Point diameter"
       << (pointLevel._diameters.size() == pointLevel._points.size() ? " Diameter\n" : "\n");
    for (size_t i = 0; i < pointLevel._points.size(); ++i) {
        os << dumpPoint(pointLevel._points[i]) << ' ' << pointLevel._diameters[i] << '\n';
    }
    return os;
}

std::ostream& operator<<(std::ostream& os, const Properties& properties) {
    return os << properties._pointLevel << '\n';
}
}  // namespace property
}  // namespace vasculature
}  // namespace morphio
