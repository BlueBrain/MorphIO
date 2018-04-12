#pragma once

#include <morphio/types.h>


namespace morphio
{
namespace Property
{
template <typename T>
void _appendVector(std::vector<T>& to, const std::vector<T>& from, int offset);

struct Section
{
    typedef Vector2i Type;
};

struct MitoSection
{
    typedef Vector2i Type;
};

struct Point
{
    typedef morphio::Point Type;
};

struct SectionType
{
    typedef morphio::SectionType Type;
};

struct Perimeter
{
    typedef float Type;
};

struct Diameter
{
    typedef float Type;
};

struct MitoPathLength
{
    typedef float Type;
};

struct MitoDiameter
{
    typedef float Type;
};

struct MitoNeuriteSectionId
{
    typedef uint32_t Type;
};

struct PointLevel
{
    std::vector<Point::Type> _points;
    std::vector<Perimeter::Type> _perimeters;
    std::vector<Diameter::Type> _diameters;

    PointLevel() {}
    PointLevel(std::vector<Point::Type> points,
               std::vector<Diameter::Type> diameters,
               std::vector<Perimeter::Type> perimeters = std::vector<Perimeter::Type>());
    PointLevel(const PointLevel& data);
    PointLevel(const PointLevel& data, SectionRange range);
    bool operator==(const PointLevel& other) const;
    bool operator!=(const PointLevel& other) const;
};

struct MitochondriaPointLevel
{
    std::vector<MitoNeuriteSectionId::Type> _sectionIds;
    std::vector<MitoPathLength::Type> _relativePathLengths;
    std::vector<MitoDiameter::Type> _diameters;

    MitochondriaPointLevel(){};

    MitochondriaPointLevel(std::vector<uint32_t> sectionId,
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
    std::map<uint32_t, std::vector<uint32_t>> _children;

    bool operator==(const SectionLevel& other) const;
    bool operator!=(const SectionLevel& other) const;
};

struct CellLevel
{
    morphio::CellFamily _cellFamily;
    SomaType _somaType;
    MorphologyVersion _version;

    bool operator==(const CellLevel& other) const;
    bool operator!=(const CellLevel& other) const;
};



// The lowest level data blob
struct Properties
{
    PointLevel _pointLevel;
    SectionLevel _sectionLevel;
    CellLevel _cellLevel;

    MitochondriaPointLevel _mitochondriaPointLevel;
    MitochondriaSectionLevel _mitochondriaSectionLevel;

    template <typename T>
    std::vector<typename T::Type>& get();
    template <typename T>
    const std::vector<typename T::Type>& get() const;

    const morphio::MorphologyVersion& version()
    {
        return _cellLevel._version;
    }
    const morphio::CellFamily& cellFamily() { return _cellLevel._cellFamily; }
    const morphio::SomaType& somaType() { return _cellLevel._somaType; }
    const std::map<uint32_t, std::vector<uint32_t>>& children()
    {
        return _sectionLevel._children;
    }
    bool operator==(const Properties& other) const;
    bool operator!=(const Properties& other) const;
};

std::ostream& operator<<(std::ostream& os, const Properties& properties);
std::ostream& operator<<(std::ostream& os, const PointLevel& pointLevel);


} // namespace Property
} // namespace morphio
