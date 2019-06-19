#include <algorithm>
#include <cmath>

#include <morphio/errorMessages.h>
#include <morphio/properties.h>
#include <morphio/shared_utils.tpp>


namespace morphio {
namespace Property {

PointLevel::PointLevel(std::vector<Point::Type> points,
    std::vector<Diameter::Type> diameters,
    std::vector<Perimeter::Type> perimeters)
    : _points(points)
    , _diameters(diameters)
    , _perimeters(perimeters)
{
    if (_points.size() != _diameters.size())
        throw SectionBuilderError(
            "Point vector have size: " + std::to_string(_points.size()) + " while Diameter vector has size: " + std::to_string(_diameters.size()));

    if (_perimeters.size() > 0 && _points.size() != _perimeters.size())
        throw SectionBuilderError(
            "Point vector have size: " + std::to_string(_points.size()) + " while Perimeter vector has size: " + std::to_string(_perimeters.size()));
}

PointLevel::PointLevel(const PointLevel& data)
    : PointLevel(data._points, data._diameters, data._perimeters)
{
}

PointLevel::PointLevel(const PointLevel& data, SectionRange range)
{
    _points = copySpan<Property::Point>(data._points, range);
    _diameters = copySpan<Property::Diameter>(data._diameters, range);
    _perimeters = copySpan<Property::Perimeter>(data._perimeters, range);
}

PointLevel& PointLevel::operator=(const PointLevel& other)
{
    if (&other == this)
        return *this;

    this->_points = other._points;
    this->_diameters = other._diameters;
    this->_perimeters = other._perimeters;
    return *this;
}

template <typename T>
bool compare(const std::vector<T>& vec1, const std::vector<T>& vec2,
    const std::string& name, LogLevel logLevel)
{
    if (vec1 == vec2)
        return true;

    if (vec1.size() != vec2.size()) {
        if (logLevel > LogLevel::ERROR)
            LBERROR(Warning::UNDEFINED,
                "Error comparing " + name + ", size differs: " + std::to_string(vec1.size()) + " vs " + std::to_string(vec2.size()));
        return false;
    }

    if (logLevel > LogLevel::ERROR) {
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

static bool compare_section_structure(const std::vector<Section::Type>& vec1,
    const std::vector<Section::Type>& vec2,
    const std::string& name, LogLevel logLevel)
{
    if (vec1.size() != vec2.size()) {
        if (logLevel > LogLevel::ERROR)
            LBERROR(Warning::UNDEFINED,
                "Error comparing " + name + ", size differs: " + std::to_string(vec1.size()) + " vs " + std::to_string(vec2.size()));
        return false;
    }

    for (unsigned int i = 1; i < vec1.size(); ++i) {
        if (vec1[i][0] - vec1[1][0] != vec2[i][0] - vec2[1][0] || vec1[i][1] != vec2[i][1]) {
            if (logLevel > LogLevel::ERROR) {
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
    const std::string& name, LogLevel logLevel)
{
    if (vec1.size() != vec2.size()) {
        if (logLevel > LogLevel::ERROR)
            LBERROR(Warning::UNDEFINED,
                "Error comparing " + name + ", size differs: " + std::to_string(vec1.size()) + " vs " + std::to_string(vec2.size()));
        return false;
    }

    const float epsilon = 1e-6f;
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
    const std::string& name, LogLevel logLevel)
{
    if (vec1.size() != vec2.size()) {
        if (logLevel > LogLevel::ERROR)
            LBERROR(Warning::UNDEFINED,
                "Error comparing " + name + ", size differs: " + std::to_string(vec1.size()) + " vs " + std::to_string(vec2.size()));
        return false;
    }

    const float epsilon = 1e-6f;
    for (unsigned int i = 0; i < vec1.size(); ++i) {
        if (std::fabs(distance(vec1[i], vec2[i])) > epsilon) {
            if (logLevel > LogLevel::ERROR) {
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
    const std::string& name, LogLevel logLevel)
{
    if (vec1 == vec2)
        return true;
    if (logLevel > LogLevel::ERROR) {
        if (vec1.size() != vec2.size()) {
            LBERROR(Warning::UNDEFINED,
                "Error comparing " + name + ", size differs: " + std::to_string(vec1.size()) + " vs " + std::to_string(vec2.size()));
        }
    }

    return false;
}

template <typename T>
bool compare(const T& el1, const T& el2, const std::string& name, LogLevel logLevel)
{
    if (el1 == el2)
        return true;

    if (logLevel > LogLevel::ERROR)
        LBERROR(Warning::UNDEFINED, name + " differs");
    return false;
}

bool SectionLevel::diff(const SectionLevel& other, LogLevel logLevel) const
{
    return !(this == &other ||
             (compare_section_structure(this->_sections, other._sections, "_sections", logLevel) &&
              compare(this->_sectionTypes, other._sectionTypes, "_sectionTypes", logLevel) &&
              compare(this->_children, other._children, "_children", logLevel)));
}

bool SectionLevel::operator==(const SectionLevel& other) const
{
    return !diff(other, LogLevel::ERROR);
}

bool SectionLevel::operator!=(const SectionLevel& other) const
{
    return diff(other, LogLevel::ERROR);
}

bool CellLevel::diff(const CellLevel& other, LogLevel logLevel) const
{
    if (logLevel && this->_cellFamily != other._cellFamily) {
        std::cout << "this->_cellFamily: " << this->_cellFamily << std::endl;
        std::cout << "other._cellFamily: " << other._cellFamily << std::endl;
    }
    return !(this == &other || (this->_cellFamily == other._cellFamily
                                 // this->_somaType == other._somaType
                                 ));
}

bool CellLevel::operator==(const CellLevel& other) const
{
    return !diff(other, LogLevel::ERROR);
}

bool CellLevel::operator!=(const CellLevel& other) const
{
    return diff(other, LogLevel::ERROR);
}


MitochondriaPointLevel::MitochondriaPointLevel(
    const MitochondriaPointLevel& data, SectionRange range)
{
    _sectionIds = copySpan<Property::MitoNeuriteSectionId>(data._sectionIds, range);
    _relativePathLengths = copySpan<Property::MitoPathLength>(data._relativePathLengths, range);
    _diameters = copySpan<Property::MitoDiameter>(data._diameters, range);
}

MitochondriaPointLevel::MitochondriaPointLevel(
    std::vector<MitoNeuriteSectionId::Type> sectionIds,
    std::vector<MitoPathLength::Type> relativePathLengths,
    std::vector<MitoDiameter::Type> diameters)
    : _sectionIds(sectionIds)
    , _relativePathLengths(relativePathLengths)
    , _diameters(diameters)
{
    if (_sectionIds.size() != _relativePathLengths.size())
        throw SectionBuilderError(
            "While building MitochondriaPointLevel:\n"
            "section IDs vector have size: " +
            std::to_string(_sectionIds.size()) + " while relative path length vector has size: " + std::to_string(_relativePathLengths.size()));

    if (_sectionIds.size() != _diameters.size())
        throw SectionBuilderError(
            "While building MitochondriaPointLevel:\n"
            "section IDs vector have size: " +
            std::to_string(_sectionIds.size()) + " while diameter vector has size: " + std::to_string(_diameters.size()));
}

bool MitochondriaSectionLevel::diff(const MitochondriaSectionLevel& other, LogLevel logLevel) const
{
    return !(this == &other ||
             (compare_section_structure(this->_sections, other._sections, "_sections", logLevel) &&
              compare(this->_children, other._children, "_children", logLevel)));
}

bool MitochondriaSectionLevel::operator==(const MitochondriaSectionLevel& other) const
{
    return !diff(other, LogLevel::ERROR);
}

bool MitochondriaSectionLevel::operator!=(const MitochondriaSectionLevel& other) const
{
    return diff(other, LogLevel::ERROR);
}

bool MitochondriaPointLevel::diff(const MitochondriaPointLevel& other, LogLevel logLevel) const
{
    return !(this == &other ||
             (compare(this->_sectionIds, other._sectionIds, "mito section ids", logLevel) &&
              compare(this->_relativePathLengths, other._relativePathLengths, "mito relative pathlength", logLevel) &&
              compare(this->_diameters, other._diameters, "mito section diameters", logLevel)));
}

bool MitochondriaPointLevel::operator==(const MitochondriaPointLevel& other) const
{
    return !diff(other, LogLevel::ERROR);
}

bool MitochondriaPointLevel::operator!=(const MitochondriaPointLevel& other) const
{
    return diff(other, LogLevel::ERROR);
}

Annotation::Annotation(AnnotationType type, uint32_t sectionId,
    PointLevel points, std::string details,
    int32_t lineNumber)
    : _type(type)
    , _sectionId(sectionId)
    , _points(points)
    , _lineNumber(lineNumber)
    , _details(details)
{
}

template <>
std::vector<Section::Type>& Properties::get<Section>()
{
    return _sectionLevel._sections;
}

template <>
const std::vector<Section::Type>& Properties::get<Section>() const
{
    return _sectionLevel._sections;
}

template <>
std::vector<MitoSection::Type>& Properties::get<MitoSection>()
{
    return _mitochondriaSectionLevel._sections;
}

template <>
const std::vector<MitoSection::Type>& Properties::get<MitoSection>() const
{
    return _mitochondriaSectionLevel._sections;
}

template <>
std::vector<MitoNeuriteSectionId::Type>& Properties::get<MitoNeuriteSectionId>()
{
    return _mitochondriaPointLevel._sectionIds;
}

template <>
const std::vector<MitoNeuriteSectionId::Type>&
    Properties::get<MitoNeuriteSectionId>() const
{
    return _mitochondriaPointLevel._sectionIds;
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
const std::vector<SectionType::Type>& Properties::get<SectionType>() const
{
    return _sectionLevel._sectionTypes;
}

template <>
std::vector<Perimeter::Type>& Properties::get<Perimeter>()
{
    return _pointLevel._perimeters;
}

template <>
const std::vector<Perimeter::Type>& Properties::get<Perimeter>() const
{
    return _pointLevel._perimeters;
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
std::vector<MitoDiameter::Type>& Properties::get<MitoDiameter>()
{
    return _mitochondriaPointLevel._diameters;
}

template <>
const std::vector<MitoDiameter::Type>& Properties::get<MitoDiameter>() const
{
    return _mitochondriaPointLevel._diameters;
}

template <>
std::vector<MitoPathLength::Type>& Properties::get<MitoPathLength>()
{
    return _mitochondriaPointLevel._relativePathLengths;
}

template <>
const std::vector<MitoPathLength::Type>& Properties::get<MitoPathLength>() const
{
    return _mitochondriaPointLevel._relativePathLengths;
}

template <>
const std::map<int32_t, std::vector<uint32_t>>& Properties::children<Section>()
{
    return _sectionLevel._children;
}

template <>
const std::map<int32_t, std::vector<uint32_t>>&
    Properties::children<MitoSection>()
{
    return _mitochondriaSectionLevel._children;
}

std::ostream& operator<<(std::ostream& os, const PointLevel& prop)
{
    os << "Point level properties:" << std::endl;
    os << "Point Diameter"
       << (prop._perimeters.size() == prop._points.size() ? " Perimeter" : "")
       << std::endl;
    for (unsigned int i = 0; i < prop._points.size(); ++i) {
        os << dumpPoint(prop._points[i]) << ' ' << prop._diameters[i];
        if (prop._perimeters.size() == prop._points.size())
            os << ' ' << prop._perimeters[i];
        os << std::endl;
    }
    return os;
}

std::ostream& operator<<(std::ostream& os, const Properties& properties)
{
    os << properties._pointLevel << std::endl;
    // os << _sectionLevel << std::endl;
    // os << _cellLevel << std::endl;
    return os;
}


} // namespace Property
} // namespace morphio
