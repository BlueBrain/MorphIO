/**
   The property namespace is the core of MorphIO as it is where all the
   internal data are stored.

   The higher level container structure is Property::Properties.

   It contains low-lever structure that stores information at various levels:
       - PointLevel: information that is available at the point level (point
 coordinate, diameter, perimeter)
       - SectionLevel: information that is available at the section level
 (section type, parent section)
       - CellLevel: information that is available at the cell level (cell type,
 file version, soma type)
       - MitochondriaPointLevel: information that is available at the
 mitochondrial point level (enclosing neuronal section, relative distance to
 start of neuronal section, diameter)
       - MitochondriaSectionLevel: information that is available at the
 mitochondrial section level (parent section)
 **/
#pragma once

#include <map>
#include <morphio/types.h>

namespace morphio {
namespace Property {

struct Section {
    // (offset, parent index)
    using Type = std::array<int, 2>;
};

struct MitoSection {
    // (offset, parent index)
    using Type = std::array<int, 2>;
};

struct Point {
    using Type = morphio::Point;
};

struct SectionType {
    using Type = morphio::SectionType;
};

struct Perimeter {
    using Type = floatType;
};

struct Diameter {
    using Type = floatType;
};

struct MitoPathLength {
    using Type = floatType;
};

struct MitoDiameter {
    using Type = floatType;
};

struct MitoNeuriteSectionId {
    using Type = uint32_t;
};

struct PointLevel {
    std::vector<Point::Type> _points;
    std::vector<Diameter::Type> _diameters;
    std::vector<Perimeter::Type> _perimeters;

    PointLevel() = default;
    PointLevel(std::vector<Point::Type> points,
               std::vector<Diameter::Type> diameters,
               std::vector<Perimeter::Type> perimeters = {});
    PointLevel(const PointLevel& data);
    PointLevel(const PointLevel& data, SectionRange range);
    PointLevel& operator=(const PointLevel& other);
    // bool operator==(const PointLevel& other) const;
    // bool operator!=(const PointLevel& other) const;
};

struct SectionLevel {
    std::vector<Section::Type> _sections;
    std::vector<SectionType::Type> _sectionTypes;
    std::map<int, std::vector<unsigned int>> _children;

    bool operator==(const SectionLevel& other) const;
    bool operator!=(const SectionLevel& other) const;
    /**
       Like operator!= but with logLevel argument
    **/
    bool diff(const SectionLevel& other, LogLevel logLevel) const;
};

struct SomaLevel {
    Section::Type _sections;
};

struct MitochondriaPointLevel {
    std::vector<MitoNeuriteSectionId::Type> _sectionIds;
    std::vector<MitoPathLength::Type> _relativePathLengths;
    std::vector<MitoDiameter::Type> _diameters;

    MitochondriaPointLevel() = default;
    MitochondriaPointLevel(const MitochondriaPointLevel& data, const SectionRange& range);

    MitochondriaPointLevel(std::vector<uint32_t> sectionId,
                           // relative pathlength between the current points
                           // and the start of the neuronal section
                           std::vector<MitoPathLength::Type> relativePathLengths,
                           std::vector<MitoDiameter::Type> diameters);

    bool diff(const MitochondriaPointLevel& other, LogLevel logLevel) const;
    bool operator==(const MitochondriaPointLevel& other) const;
    bool operator!=(const MitochondriaPointLevel& other) const;
};

struct MitochondriaSectionLevel {
    std::vector<Section::Type> _sections;
    std::map<int, std::vector<unsigned int>> _children;

    bool diff(const MitochondriaSectionLevel& other, LogLevel logLevel) const;
    bool operator==(const MitochondriaSectionLevel& other) const;
    bool operator!=(const MitochondriaSectionLevel& other) const;
};

struct EndoplasmicReticulumLevel {
    std::vector<uint32_t> _sectionIndices;
    std::vector<morphio::floatType> _volumes;
    std::vector<morphio::floatType> _surfaceAreas;
    std::vector<uint32_t> _filamentCounts;
};

struct Annotation {
    Annotation(AnnotationType type,
               uint32_t sectionId,
               PointLevel points,
               std::string details,
               int32_t lineNumber);

    AnnotationType _type;
    uint32_t _sectionId;
    PointLevel _points;
    int32_t _lineNumber;
    std::string _details;
};

struct Marker {
    PointLevel _pointLevel;
    std::string _label;
};

struct CellLevel {
    CellLevel()
        : _version({"undefined", 0, 0}) {}

    // A tuple (file format (std::string), major version, minor version)
    MorphologyVersion _version;
    morphio::CellFamily _cellFamily;
    SomaType _somaType;
    std::vector<Annotation> _annotations;
    std::vector<Marker> _markers;

    bool diff(const CellLevel& other, LogLevel logLevel) const;
    bool operator==(const CellLevel& other) const;
    bool operator!=(const CellLevel& other) const;
    std::string fileFormat() const;
    uint32_t majorVersion();
    uint32_t minorVersion();
};

// The lowest level data blob
struct Properties {
    ////////////////////////////////////////////////////////////////////////////////
    // Data stuctures
    ////////////////////////////////////////////////////////////////////////////////

    PointLevel _pointLevel;
    SectionLevel _sectionLevel;
    CellLevel _cellLevel;
    PointLevel _somaLevel;

    MitochondriaPointLevel _mitochondriaPointLevel;
    MitochondriaSectionLevel _mitochondriaSectionLevel;

    EndoplasmicReticulumLevel _endoplasmicReticulumLevel;

    ////////////////////////////////////////////////////////////////////////////////
    // Functions
    ////////////////////////////////////////////////////////////////////////////////
    template <typename T>
    std::vector<typename T::Type>& get() noexcept;
    template <typename T>
    const std::vector<typename T::Type>& get() const noexcept;

    const morphio::MorphologyVersion& version() const noexcept {
        return _cellLevel._version;
    }
    const morphio::CellFamily& cellFamily() const noexcept {
        return _cellLevel._cellFamily;
    }
    const morphio::SomaType& somaType() const noexcept {
        return _cellLevel._somaType;
    }
    template <typename T>
    const std::map<int32_t, std::vector<uint32_t>>& children() const noexcept;
};

template <>
const std::map<int32_t, std::vector<uint32_t>>& Properties::children<Section>() const noexcept;
template <>
const std::map<int32_t, std::vector<uint32_t>>& Properties::children<MitoSection>() const noexcept;

std::ostream& operator<<(std::ostream& os, const Properties& properties);
std::ostream& operator<<(std::ostream& os, const PointLevel& pointLevel);

template <>
const std::vector<Point::Type>& Properties::get<Point>() const noexcept;
template <>
std::vector<Point::Type>& Properties::get<Point>() noexcept;

template <>
const std::vector<Perimeter::Type>& Properties::get<Perimeter>() const noexcept;
template <>
std::vector<Perimeter::Type>& Properties::get<Perimeter>() noexcept;

template <>
const std::vector<Diameter::Type>& Properties::get<Diameter>() const noexcept;
template <>
std::vector<Diameter::Type>& Properties::get<Diameter>() noexcept;

template <>
const std::vector<MitoSection::Type>& Properties::get<MitoSection>() const noexcept;
template <>
std::vector<MitoSection::Type>& Properties::get<MitoSection>() noexcept;

template <>
const std::vector<MitoPathLength::Type>& Properties::get<MitoPathLength>() const noexcept;
template <>
std::vector<MitoPathLength::Type>& Properties::get<MitoPathLength>() noexcept;

template <>
const std::vector<MitoNeuriteSectionId::Type>& Properties::get<MitoNeuriteSectionId>() const
    noexcept;
template <>
std::vector<MitoNeuriteSectionId::Type>& Properties::get<MitoNeuriteSectionId>() noexcept;

template <>
const std::vector<MitoDiameter::Type>& Properties::get<MitoDiameter>() const noexcept;
template <>
std::vector<MitoDiameter::Type>& Properties::get<MitoDiameter>() noexcept;

template <>
const std::vector<Section::Type>& Properties::get<Section>() const noexcept;
template <>
std::vector<Section::Type>& Properties::get<Section>() noexcept;

template <>
const std::vector<SectionType::Type>& Properties::get<SectionType>() const noexcept;
template <>
std::vector<SectionType::Type>& Properties::get<SectionType>() noexcept;

}  // namespace Property
}  // namespace morphio
