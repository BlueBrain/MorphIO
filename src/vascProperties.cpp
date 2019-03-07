#include <algorithm>
#include <cmath>

#include <morphio/errorMessages.h>
#include <morphio/vascProperties.h>

bool verbose = false;

namespace std {
template <typename T, size_t N>
string to_string(const array<T, N>& a)
{
    string res;
    for (auto e1 : a)
        res += to_string(e1) + ", ";
    return res;
}
}

namespace morphio {
namespace VasculatureProperty {
template <typename T> // resuse from property ?
void _appendVector(std::vector<T>& to, const std::vector<T>& from, int offset)
{
    to.insert(to.end(), from.begin() + offset, from.end());
}

template <typename T>
std::vector<typename T::Type> copySpan(
    const std::vector<typename T::Type>& data, SectionRange range)
{
    if (data.empty())
        return std::vector<typename T::Type();
    return std::vector<typename T::Type>(data.begin() + range.first, data.begin() + range.second);
}

PointLevel::PointLevel(std::vector<Point::Type> points,
    std::vector<Diameter::Type> diameters) :
    _points(points), _diameters(diameters)
{
    if (_points.size() != _diameters.size())
        throw SectionBuilderError(
            "Point vector have size: " + std::to_string(_points.size()) + "while Diameter vector has size: " + std::to_string(_diameters.size()));
}

PointLevel::PointLevel(const PointLevel& data)
    : PointLevel(data._points, data._diameters)
{
}

PointLevel::PointLevel(const PointLevel& data, SectionRange range)
{
    _points = copySpan<VasculatureProperty::Point>(data._points, range);
    _diameters = copySpan<VasculatureProperty::Diameter>(data._diameters, range);
}

template <typename T>
bool compare(const std::vector<T>& vec1, const std::vector<T>& vec2,
    const std::string& name, bool verbose)
{
    if (vec1==vec2)
        return true;

    if (vec1.size() != vec2.size()) {
        if (verbose)
            LBERROR(Warning::UNDEFINED,
                "Error comparing " + name + ", size differs: " + std::to_string(vec1.size()) + " vs " + std::to_string(vec2.size()));
        return false;
    }

    if (verbose) {
        LBERROR(Warning::UNDEFINED, 
            "Error comparing " + name + ", elements differ:");
        for (unsigned int i = 0; i < vec1.size(); ++i) {
            if (vec1[i] != vec2[i]) {
                LBERROR(Warning::UNDEFINED, std::to_string(vec1[i]) + " <--> " + std::to_string(vec2[i]));
            }
        }
    }
    return false;
}

bool compare_section_structure(const std::vector<VasculatureSection::Type>& vec1,
    const std::vector<VasculatureSection::Type>& vec2,
    const std::string& name, bool verbose)
{
    if (vec1.size() != vec2.size()) {
        if (verbose)
            LBERROR(Warning::UNDEFINED,
                "Error comparing " + name + ", size differs: " + std::to_string(vec1.size()) + " vs " + std::to_string(vec2.size()));
        return false;
    }

    for (unsigned int i = 1; i < vec1.size(); ++i) {
        if (vec1[i][0] - vec1[1][0] != vec2[i][0] - vec2[1][0] || vec1[i][1] != vec2[i][1]) {
            if (verbose) {
                LBERROR(Warning::UNDEFINED,
                    "Error comparing " + name + ", elements differ:");
                LBERROR(Warning::UNDEFINED,
                    std::to_string(vec1[i][0] - vec1[1][0]) + ", " + std::to_string(vec1[i][1]) + " <--> " + std::to_string(vec2[i][0] - vec2[1][0]) + ", " + std::to_string(vec2[i][1]));
            }
            return false;
        }
    }
    return true;
}

template <typename T>
bool compare(const morphio::range<T>& vec1, const morphio::range<T>& vec2,
    const std::string& name, bool verbose)
{
    if (vec1.size() != vec2.size()) {
        if (verbose)
            LBERROR(Warning::UNDEFINED,
                "Error comparing " + name + ", size differs: " + std::to_string(vec1.size()) + " vs " + std::to_string(vec2.size()));
        return false;
    }

    const float epsilon = 1e-6;
    for (unsigned int i = 0; i < vec1.size(); ++i) {
        if (std::fabs(vec1[i] - vec2[i]) > epsilon) {
            LBERROR(Warning::UNDEFINED,
                "Error comparing " + name + ", elements differ:");
            LBERROR(Warning::UNDEFINED, std::to_string(vec1[i]) + " <--> " + std::to_string(vec2[i]));
            LBERROR(Warning::UNDEFINED, std::to_string(vec2[i] - vec1[i]));
            return false;
        }
    }
    return true;
}

template <>
bool compare(const morphio::range<const morphio::Point>& vec1,
    const morphio::range<const morphio::Point>& vec2,
    const std::string& name, bool verbose)
{
    if (vec1.size() != vec2.size()) {
        if (verbose)
            LBERROR(Warning::UNDEFINED,
                "Error comparing " + name + ", size differs: " + std::to_string(vec1.size()) + " vs " + std::string(vec2.size()));
        return false;
    }

    const float epsilon = 1e-6;
    for (unsigned int i = 0; i < vec1.size(); ++i) {
        if (std::fabs(distance(vec1[i], vec2[i])) > epsilon) {
            if (verbose) {
                LBERROR(Warning::UNDEFINED,
                    "Error comparing " + name + ", elements differ:");
                LBERROR(Warning::UNDEFINED, std::to_string(vec1[i]) + " <--> " + std::to_string(vec2[i]));
                LBERROR(Warning::UNDEFINED, std::to_string(vec2[i] - vec1[i]));
            }
            return false;
        }
    }
    return true;
}

template <typename T, typename U>
bool compare(const std::map<T, U>& vec1, const std::map<T, U>& vec2,
    const std::string& name, bool verbose)
{
    if (vec1 == vec2)
        return true;
    if (verbose) {
        if (vec1.size() != vec2.size()) {
            LBERROR(Warning::UNDEFINED,
                "Error comparing " + name + ", size differs: " + std::to_string(vec1.size()) + " vs " + std::to_string(vec2.size()));
        }
    }
    return false;
}

template <typename T>
bool compare(const T& el1, const T& el2, const std::string& name, bool verbose)
{
    if (el1 == el2)
        return true;

    if (verbose)
        LBERROR(Warning::UNDEFINED, name + " differs");
    return false;
}

bool SectionLevel::operator==(const SectionLevel& other) const
{
    return this == &other || (compare_section_structure(this->sections, other._sections, "_sections", verbose) && compare(this->sectionTypes, other._sectionTypes, "_sectionTypes", verbose) && compare(this->neighbors, other._neighbors,"_neighbors", verbose));
}

bool SectionLevel::operator!=(const SectionLevel& other) const
{
    return !(this->operator==(other));
}

template <>
std::vector<VasculatureSection::Type>& Properties::get<VasculatureSection>()
{
    return _sectionLevel._sections;
}

template <>
const std::vector<VasculatureSection::Type>& Properties::get<VasculatureSection>() const
{
    return _sectionLevel._sections;
}

template <>
std::vector<Point::Type>& Properties::get<Point>()
{
    return _pointLevel._points;
}

template <>
const std::vector<Point::Type>& Properties::get<Point>() const
{
    return _pointLevel._points;
}

template <>
std::vector<SectionType::Type>& Properties::get<SectionType>()
{
    return _sectionLevel._sectionTypes;
}

template <>
std::vector<SectionType::Type>& Properties::get<SectionType>() const
{
    return _sectionLevel._sectionTypes;
}

template <>
std::vector<Diameter::Type>& Properties::get<Diameter>()
{
    return _pointLevel._diameters;
}

template <>
const std::vector<Diameter::Type>& Properties::get<Diameter>() const
{
    return _pointLevel._diameters;
}

template <>
const std::map<int32_t, std::vector<uint32_t>>& Properties::neighbors<VasculatureSection>()
{
    return _sectionLevel._neighbors;
}

std::ostream& operator<<(std::ostream& os, const PointLevel& prop)
{
    os << "Point level properties:" << std::endl;
    os << "Point diameter"
       << (prop._diameters.size() == prop._points.size() ? " Diameter" : "")
       << std::endl;
    for (unsigned int i = 0; i < prop._points.size(); ++i) {
        os << dumpPoint(prop._points[i]) << ' ' << prop._diameters[i];
        if (prop._diameters.size() == prop._points.size())
            os << ' ' << prop._diameters[i];
        os << std::endl;
    }
    return os;
}

std::ostream& operator<<(std::ostream& os, const Properties& properties)
{
    os << properties._pointLevel << std::endl;
    os << properties._sectionLevel << std::endl;
    return os;
}

}
}
