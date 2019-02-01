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

#include <morphio/types.h>

namespace morphio {
namespace Property {
template <typename T>
void _appendVector(std::vector<T>& to, const std::vector<T>& from, int offset);

struct Section
{
    // (offset, parent index)
    using Type = std::array<int, 2>;
};

struct MitoSection
{
    // (offset, parent index)
    using Type = std::array<int, 2>;
};

struct Point
{
    using Type = morphio::Point;
};

struct SectionType
{
    using Type = morphio::SectionType;
};

struct Perimeter
{
    using Type = float;
};

struct Diameter
{
    using Type = float;
};

struct MitoPathLength
{
    using Type = float;
};

struct MitoDiameter
{
    using Type = float;
};

struct MitoNeuriteSectionId
{
    using Type = uint32_t;
};

struct PointLevel
{
    std::vector<Point::Type> _points;
    std::vector<Diameter::Type> _diameters;
    std::vector<Perimeter::Type> _perimeters;

    PointLevel() {}
    PointLevel(std::vector<Point::Type> points,
        std::vector<Diameter::Type> diameters,
        std::vector<Perimeter::Type> perimeters = std::vector<Perimeter::Type>());
    PointLevel(const PointLevel& data);
    PointLevel(const PointLevel& data, SectionRange range);
    // bool operator==(const PointLevel& other) const;
    // bool operator!=(const PointLevel& other) const;
};

struct MitochondriaPointLevel
{
    std::vector<MitoNeuriteSectionId::Type> _sectionIds;
    std::vector<MitoPathLength::Type> _relativePathLengths;
    std::vector<MitoDiameter::Type> _diameters;

    MitochondriaPointLevel(){};
    MitochondriaPointLevel(const MitochondriaPointLevel& data,
        SectionRange range);

    MitochondriaPointLevel(
        std::vector<uint32_t> sectionId,
        // relative pathlength between the current points
        // and the start of the neuronal section
        std::vector<MitoPathLength::Type> relativePathLengths,
        std::vector<MitoDiameter::Type> diameters);
};

struct MitochondriaSectionLevel
{
    std::vector<Section::Type> _sections;
    std::map<int32_t, std::vector<uint32_t>> _children;

    bool operator==(const MitochondriaSectionLevel& other) const;
    bool operator!=(const MitochondriaSectionLevel& other) const;
};

struct SectionLevel
{
    std::vector<Section::Type> _sections;
    std::vector<SectionType::Type> _sectionTypes;
    std::map<int32_t, std::vector<uint32_t>> _children;

    bool operator==(const SectionLevel& other) const;
    bool operator!=(const SectionLevel& other) const;
};

struct SomaLevel
{
    Section::Type _sections;
};

struct Annotation
{
    Annotation(AnnotationType type, uint32_t sectionId, PointLevel points,
        std::string details, int32_t lineNumber);
    AnnotationType _type;
    uint32_t _sectionId;
    PointLevel _points;
    int32_t _lineNumber;
    std::string _details;
};

struct CellLevel
{
    morphio::CellFamily _cellFamily;
    SomaType _somaType;
    MorphologyVersion _version;
    std::vector<Annotation> annotation;

    bool operator==(const CellLevel& other) const;
    bool operator!=(const CellLevel& other) const;
};

// The lowest level data blob
struct Properties
{
    ////////////////////////////////////////////////////////////////////////////////
    // Data stuctures
    ////////////////////////////////////////////////////////////////////////////////

    PointLevel _pointLevel;
    SectionLevel _sectionLevel;
    CellLevel _cellLevel;
    PointLevel _somaLevel;

    MitochondriaPointLevel _mitochondriaPointLevel;
    MitochondriaSectionLevel _mitochondriaSectionLevel;

    std::vector<Annotation> _annotations;

    ////////////////////////////////////////////////////////////////////////////////
    // Functions
    ////////////////////////////////////////////////////////////////////////////////
    template <typename T>
    std::vector<typename T::Type>& get();
    template <typename T>
    const std::vector<typename T::Type>& get() const;

    const morphio::MorphologyVersion& version() { return _cellLevel._version; }
    const morphio::CellFamily& cellFamily() { return _cellLevel._cellFamily; }
    const morphio::SomaType& somaType() { return _cellLevel._somaType; }

    template <typename T>
    const std::map<int32_t, std::vector<uint32_t>>& children();

    bool operator==(const Properties& other) const;
    bool operator!=(const Properties& other) const;
};

template <>
const std::map<int32_t, std::vector<uint32_t>>& Properties::children<Section>();
template <>
const std::map<int32_t, std::vector<uint32_t>>&
    Properties::children<MitoSection>();

std::ostream& operator<<(std::ostream& os, const Properties& properties);
std::ostream& operator<<(std::ostream& os, const PointLevel& pointLevel);

template <>
std::vector<Point::Type>& Properties::get<Point>();
template <>
std::vector<Section::Type>& Properties::get<Section>();
template <>
const std::vector<Section::Type>& Properties::get<Section>() const;

} // namespace Property
} // namespace morphio
