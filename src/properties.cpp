/* Copyright (c) 2013-2023, EPFL/Blue Brain Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <morphio/errorMessages.h>
#include <morphio/properties.h>
#include <morphio/vector_types.h>

#include "point_utils.h"
#include "shared_utils.hpp"

namespace {

bool compare_section_structure(const std::vector<morphio::Property::Section::Type>& vec1,
                               const std::vector<morphio::Property::Section::Type>& vec2) {
    if (vec1.size() != vec2.size()) {
        return false;
    }

    for (unsigned int i = 1; i < vec1.size(); ++i) {
        if (vec1[i][0] - vec1[1][0] != vec2[i][0] - vec2[1][0] || vec1[i][1] != vec2[i][1]) {
            return false;
        }
    }

    return true;
}

}  // namespace


namespace morphio {
namespace Property {

PointLevel::PointLevel(std::vector<Point::Type> points,
                       std::vector<Diameter::Type> diameters,
                       std::vector<Perimeter::Type> perimeters)
    : _points(std::move(points))
    , _diameters(std::move(diameters))
    , _perimeters(std::move(perimeters)) {
    if (_points.size() != _diameters.size()) {
        throw SectionBuilderError(
            "Point vector have size: " + std::to_string(_points.size()) +
            " while Diameter vector has size: " + std::to_string(_diameters.size()));
    }

    if (!_perimeters.empty() && _points.size() != _perimeters.size()) {
        throw SectionBuilderError(
            "Point vector have size: " + std::to_string(_points.size()) +
            " while Perimeter vector has size: " + std::to_string(_perimeters.size()));
    }
}

PointLevel::PointLevel(const PointLevel& data)
    : PointLevel(data._points, data._diameters, data._perimeters) {}

PointLevel::PointLevel(const PointLevel& data, SectionRange range) {
    _points = copySpan<Property::Point>(data._points, range);
    _diameters = copySpan<Property::Diameter>(data._diameters, range);
    _perimeters = copySpan<Property::Perimeter>(data._perimeters, range);
}

PointLevel& PointLevel::operator=(const PointLevel& other) {
    if (&other == this) {
        return *this;
    }

    _points = other._points;
    _diameters = other._diameters;
    _perimeters = other._perimeters;

    return *this;
}

bool SectionLevel::diff(const SectionLevel& other) const {
    return !(this == &other || (compare_section_structure(_sections, other._sections) &&
                                morphio::property::compare(_sectionTypes, other._sectionTypes) &&
                                morphio::property::compare(_children, other._children)));
}

bool SectionLevel::operator==(const SectionLevel& other) const {
    return !diff(other);
}

bool SectionLevel::operator!=(const SectionLevel& other) const {
    return diff(other);
}

bool CellLevel::diff(const CellLevel& other) const {
    if (this == &other) {
        return false;
    }

    return !(_cellFamily == other._cellFamily && _somaType == other._somaType);
}

bool CellLevel::operator==(const CellLevel& other) const {
    return !diff(other);
}

bool CellLevel::operator!=(const CellLevel& other) const {
    return diff(other);
}

MitochondriaPointLevel::MitochondriaPointLevel(const MitochondriaPointLevel& data,
                                               const SectionRange& range) {
    _sectionIds = copySpan<Property::MitoNeuriteSectionId>(data._sectionIds, range);
    _relativePathLengths = copySpan<Property::MitoPathLength>(data._relativePathLengths, range);
    _diameters = copySpan<Property::MitoDiameter>(data._diameters, range);
}

MitochondriaPointLevel::MitochondriaPointLevel(
    std::vector<MitoNeuriteSectionId::Type> sectionIds,
    std::vector<MitoPathLength::Type> relativePathLengths,
    std::vector<MitoDiameter::Type> diameters)
    : _sectionIds(std::move(sectionIds))
    , _relativePathLengths(std::move(relativePathLengths))
    , _diameters(std::move(diameters)) {
    if (_sectionIds.size() != _relativePathLengths.size()) {
        throw SectionBuilderError(
            "While building MitochondriaPointLevel:\n"
            "section IDs vector have size: " +
            std::to_string(_sectionIds.size()) + " while relative path length vector has size: " +
            std::to_string(_relativePathLengths.size()));
    }

    if (_sectionIds.size() != _diameters.size()) {
        throw SectionBuilderError(
            "While building MitochondriaPointLevel:\n"
            "section IDs vector have size: " +
            std::to_string(_sectionIds.size()) +
            " while diameter vector has size: " + std::to_string(_diameters.size()));
    }
}

bool MitochondriaSectionLevel::diff(const MitochondriaSectionLevel& other) const {
    return !(this == &other || (compare_section_structure(this->_sections, other._sections) &&
                                morphio::property::compare(this->_children, other._children)));
}

bool MitochondriaSectionLevel::operator==(const MitochondriaSectionLevel& other) const {
    return !diff(other);
}

bool MitochondriaSectionLevel::operator!=(const MitochondriaSectionLevel& other) const {
    return diff(other);
}

bool MitochondriaPointLevel::diff(const MitochondriaPointLevel& other) const {
    return !(this == &other ||
             (morphio::property::compare(this->_sectionIds, other._sectionIds) &&
              morphio::property::compare(this->_relativePathLengths, other._relativePathLengths) &&
              morphio::property::compare(this->_diameters, other._diameters)));
}

bool MitochondriaPointLevel::operator==(const MitochondriaPointLevel& other) const {
    return !diff(other);
}

bool MitochondriaPointLevel::operator!=(const MitochondriaPointLevel& other) const {
    return diff(other);
}

std::ostream& operator<<(std::ostream& os, const PointLevel& pointLevel) {
    os << "Point level properties:\n"
       << "Point Diameter"
       << (pointLevel._perimeters.size() == pointLevel._points.size() ? " Perimeter\n" : "\n");
    for (unsigned int i = 0; i < pointLevel._points.size(); ++i) {
        os << dumpPoint(pointLevel._points[i]) << ' ' << pointLevel._diameters[i];
        if (pointLevel._perimeters.size() == pointLevel._points.size()) {
            os << ' ' << pointLevel._perimeters[i];
        }
        os << '\n';
    }
    return os;
}

std::ostream& operator<<(std::ostream& os, const Properties& properties) {
    os << properties._pointLevel << '\n';
    // os << _sectionLevel << '\n';
    // os << _cellLevel << '\n';
    return os;
}

}  // namespace Property
}  // namespace morphio
