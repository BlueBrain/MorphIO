#include <algorithm>
#include <cmath>

#include <morphio/errorMessages.h>
#include <morphio/properties.h>

bool verbose = false;

namespace std {
template <typename T, size_t N>
string to_string(const array<T, N>& a)
{
    string res;
    for (auto el : a)
        res += to_string(el) + ", ";
    return res;
}
} // namespace std

namespace morphio {
namespace Property {
template <typename T>
void _appendVector(std::vector<T>& to, const std::vector<T>& from, int offset)
{
    to.insert(to.end(), from.begin() + offset, from.end());
}

template <typename T>
std::vector<typename T::Type> copySpan(
    const std::vector<typename T::Type>& data, SectionRange range)
{
    if (data.empty())
        return std::vector<typename T::Type>();
    return std::vector<typename T::Type>(data.begin() + range.first,
        data.begin() + range.second);
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

template <typename T>
bool compare(const std::vector<T>& vec1, const std::vector<T>& vec2,
    const std::string& name, bool verbose)
{
    if (vec1 == vec2)
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

bool compare_section_structure(const std::vector<Section::Type>& vec1,
    const std::vector<Section::Type>& vec2,
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
                "Error comparing " + name + ", size differs: " + std::to_string(vec1.size()) + " vs " + std::to_string(vec2.size()));
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

bool compare(const PointLevel& el1, const PointLevel& el2, size_t soma_offset1,
    size_t soma_offset2, const std::string& name, bool verbose)
{
    if (&el1 == &el2)
        return true;

    // auto ptr_start = _properties->get<TProperty>().data() + _range.first;
    morphio::range<const typename Point::Type> points1(
        el1._points.data() + soma_offset1, el1._points.size() - soma_offset1);
    morphio::range<const typename Point::Type> points2(
        el2._points.data() + soma_offset2, el2._points.size() - soma_offset2);

    morphio::range<const typename Diameter::Type> diameters1(
        el1._diameters.data() + soma_offset1,
        el1._diameters.size() - soma_offset1);
    morphio::range<const typename Diameter::Type> diameters2(
        el2._diameters.data() + soma_offset2,
        el2._diameters.size() - soma_offset2);

    bool result = (compare(points1, points2, "_points", verbose) && compare(diameters1, diameters2, "_diameters", verbose));

    if (el1._perimeters.size() > soma_offset1 && el2._perimeters.size() > soma_offset2) {
        if ((el1._perimeters.size() - soma_offset1) != (el2._perimeters.size() - soma_offset2))
            return false;

        morphio::range<const typename Perimeter::Type> perimeters1(
            el1._perimeters.data() + soma_offset1,
            el1._perimeters.size() - soma_offset1);
        morphio::range<const typename Perimeter::Type> perimeters2(
            el2._perimeters.data() + soma_offset2,
            el2._perimeters.size() - soma_offset2);

        result *= compare(perimeters1, perimeters2, "_perimeters", verbose);
    }
    if (!result && verbose)
        LBERROR(Warning::UNDEFINED, "Error comparing " + name);

    return result;
}

bool SectionLevel::operator==(const SectionLevel& other) const
{
    return this == &other || (compare_section_structure(this->_sections, other._sections, "_sections", verbose) && compare(this->_sectionTypes, other._sectionTypes, "_sectionTypes", verbose) && compare(this->_children, other._children, "_children", verbose));
}

bool SectionLevel::operator!=(const SectionLevel& other) const
{
    return !(this->operator==(other));
}

bool CellLevel::operator==(const CellLevel& other) const
{
    if (verbose && this->_cellFamily != other._cellFamily) {
        std::cout << "this->_cellFamily: " << this->_cellFamily << std::endl;
        std::cout << "other._cellFamily: " << other._cellFamily << std::endl;
    }
    return this == &other || (this->_cellFamily == other._cellFamily
                                 // this->_somaType == other._somaType
                             );
}

bool CellLevel::operator!=(const CellLevel& other) const
{
    return !(this->operator==(other));
}

Annotation::Annotation(AnnotationType type, uint32_t sectionId,
    PointLevel points, std::string details,
    int32_t lineNumber)
    : _type(type)
    , _sectionId(sectionId)
    , _points(points)
    , _lineNumber(lineNumber)
    , _details(details){};

bool Properties::operator==(const Properties& other) const
{
    if (this == &other)
        return true;

    size_t this_soma_offset = get<Section>().size() > 1 ? get<Section>()[1][0] : 0;
    size_t other_soma_offset = other.get<Section>().size() > 1 ? other.get<Section>()[1][0] : 0;
    return (compare(this->_pointLevel, other._pointLevel, this_soma_offset,
                other_soma_offset, "_pointLevel", verbose) &&
            compare(this->_sectionLevel, other._sectionLevel, "_sectionLevel",
                verbose) &&
            compare(this->_cellLevel, other._cellLevel, "_cellLevel", verbose));
}

bool Properties::operator!=(const Properties& other) const
{
    return !this->operator==(other);
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

template void _appendVector(std::vector<float>&, std::vector<float> const&,
    int);
template void _appendVector(std::vector<std::array<float, 3ul>>&,
    std::vector<std::array<float, 3ul>> const&, int);
template void _appendVector(std::vector<unsigned int>&,
    std::vector<unsigned int> const&, int);

} // namespace Property
} // namespace morphio
