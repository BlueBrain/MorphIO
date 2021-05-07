#include <algorithm>
#include <cmath>

#include <morphio/errorMessages.h>
#include <morphio/properties.h>
#include <morphio/vasc/properties.h>

#include "../shared_utils.hpp"

namespace morphio {

namespace vasculature {
static bool verbose = false;
namespace property {

VascPointLevel::VascPointLevel(const std::vector<Point::Type>& points,
                               const std::vector<Diameter::Type>& diameters)
    : _points(points)
    , _diameters(diameters) {
    if (_points.size() != _diameters.size())
        throw SectionBuilderError(
            "Point vector have size: " + std::to_string(_points.size()) +
            "while Diameter vector has size: " + std::to_string(_diameters.size()));
}

VascPointLevel::VascPointLevel(const VascPointLevel& data)
    : VascPointLevel(data._points, data._diameters) {}

VascPointLevel::VascPointLevel(const VascPointLevel& data, SectionRange range) {
    _points = copySpan<property::Point>(data._points, range);
    _diameters = copySpan<property::Diameter>(data._diameters, range);
}

template <typename T>
bool compare(const std::vector<T>& vec1,
             const std::vector<T>& vec2,
             const std::string& name,
             bool verbose_) {
    if (vec1 == vec2)
        return true;

    if (vec1.size() != vec2.size()) {
        if (verbose_)
            printError(Warning::UNDEFINED,
                       "Error comparing " + name + ", size differs: " +
                           std::to_string(vec1.size()) + " vs " + std::to_string(vec2.size()));
        return false;
    }

    if (verbose_) {
        printError(Warning::UNDEFINED, "Error comparing " + name + ", elements differ:");
        for (size_t i = 0; i < vec1.size(); ++i) {
            if (vec1[i] != vec2[i]) {
                printError(Warning::UNDEFINED,
                           valueToString(vec1[i]) + " <--> " + valueToString(vec2[i]));
            }
        }
    }
    return false;
}

static bool compare_section_structure(const std::vector<VascSection::Type>& vec1,
                                      const std::vector<VascSection::Type>& vec2,
                                      const std::string& name,
                                      bool verbose_) {
    if (vec1.size() != vec2.size()) {
        if (verbose_)
            printError(Warning::UNDEFINED,
                       "Error comparing " + name + ", size differs: " +
                           std::to_string(vec1.size()) + " vs " + std::to_string(vec2.size()));
        return false;
    }

    for (size_t i = 1; i < vec1.size(); ++i) {
        if (vec1[i] - vec1[1] != vec2[i] - vec2[1]) {
            if (verbose_) {
                printError(Warning::UNDEFINED, "Error comparing " + name + ", elements differ:");
                printError(Warning::UNDEFINED,
                           std::to_string(vec1[i] - vec1[1]) + " <--> " +
                               std::to_string(vec2[i] - vec2[1]));
            }
            return false;
        }
    }
    return true;
}

template <typename T>
bool compare(const morphio::range<T>& vec1,
             const morphio::range<T>& vec2,
             const std::string& name,
             bool verbose_) {
    if (vec1.size() != vec2.size()) {
        if (verbose_)
            printError(Warning::UNDEFINED,
                       "Error comparing " + name + ", size differs: " +
                           std::to_string(vec1.size()) + " vs " + std::to_string(vec2.size()));
        return false;
    }

    for (size_t i = 0; i < vec1.size(); ++i) {
        if (std::fabs(vec1[i] - vec2[i]) > morphio::epsilon) {
            printError(Warning::UNDEFINED, "Error comparing " + name + ", elements differ:");
            printError(Warning::UNDEFINED,
                       valueToString(vec1[i]) + " <--> " + valueToString(vec2[i]));
            printError(Warning::UNDEFINED, valueToString(vec2[i] - vec1[i]));
            return false;
        }
    }
    return true;
}

template <>
bool compare(const morphio::range<const morphio::Point>& vec1,
             const morphio::range<const morphio::Point>& vec2,
             const std::string& name,
             bool verbose_) {
    if (vec1.size() != vec2.size()) {
        if (verbose_)
            printError(Warning::UNDEFINED,
                       "Error comparing " + name + ", size differs: " +
                           std::to_string(vec1.size()) + " vs " + std::to_string(vec2.size()));
        return false;
    }

    for (size_t i = 0; i < vec1.size(); ++i) {
        if (std::fabs(distance(vec1[i], vec2[i])) > morphio::epsilon) {
            if (verbose_) {
                printError(Warning::UNDEFINED, "Error comparing " + name + ", elements differ:");
                printError(Warning::UNDEFINED,
                           valueToString(vec1[i]) + " <--> " + valueToString(vec2[i]));
                printError(Warning::UNDEFINED, valueToString(vec2[i] - vec1[i]));
            }
            return false;
        }
    }
    return true;
}

template <typename T, typename U>
bool compare(const std::map<T, U>& vec1,
             const std::map<T, U>& vec2,
             const std::string& name,
             bool verbose_) {
    if (vec1 == vec2)
        return true;
    if (verbose_) {
        if (vec1.size() != vec2.size()) {
            printError(Warning::UNDEFINED,
                       "Error comparing " + name + ", size differs: " +
                           std::to_string(vec1.size()) + " vs " + std::to_string(vec2.size()));
        }
    }
    return false;
}

template <typename T>
bool compare(const T& el1, const T& el2, const std::string& name, bool verbose_) {
    if (el1 == el2)
        return true;

    if (verbose_)
        printError(Warning::UNDEFINED, name + " differs");
    return false;
}

static bool compare(const VascPointLevel& el1,
                    const VascPointLevel& el2,
                    const std::string& name,
                    bool verbose_) {
    if (&el1 == &el2)
        return true;

    bool result = (compare(el1._points, el2._points, "_points", verbose_) &&
                   compare(el1._diameters, el2._diameters, "_diameters", verbose_));

    if (!result && verbose_)
        printError(Warning::UNDEFINED, "Error comparing " + name);

    return result;
}

bool VascSectionLevel::operator==(const VascSectionLevel& other) const {
    return this == &other ||
           (compare_section_structure(this->_sections, other._sections, "_sections", verbose) &&
            compare(this->_sectionTypes, other._sectionTypes, "_sectionTypes", verbose) &&
            compare(this->_predecessors, other._predecessors, "_predecessors", verbose) &&
            compare(this->_successors, other._successors, "_successors", verbose));
}

bool VascSectionLevel::operator!=(const VascSectionLevel& other) const {
    return !(this->operator==(other));
}

bool Properties::operator==(const Properties& other) const {
    if (this == &other)
        return true;

    return (compare(this->_pointLevel, other._pointLevel, "_pointLevel", verbose) &&
            compare(this->_sectionLevel, other._sectionLevel, "_sectionLevel", verbose));
}

bool Properties::operator!=(const Properties& other) const {
    return !this->operator==(other);
}

std::ostream& operator<<(std::ostream& os, const VascPointLevel& prop) {
    os << "Point level properties:\n";
    os << "Point diameter"
       << (prop._diameters.size() == prop._points.size() ? " Diameter\n" : "\n");
    for (size_t i = 0; i < prop._points.size(); ++i) {
        os << dumpPoint(prop._points[i]) << ' ' << prop._diameters[i] << '\n';
    }
    return os;
}

std::ostream& operator<<(std::ostream& os, const Properties& properties) {
    return os << properties._pointLevel << '\n';
}
}  // namespace property
}  // namespace vasculature
}  // namespace morphio
