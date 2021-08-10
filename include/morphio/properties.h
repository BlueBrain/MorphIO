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
};

struct SectionLevel {
    std::vector<Section::Type> _sections;
    std::vector<SectionType::Type> _sectionTypes;
    std::map<int, std::vector<unsigned int>> _children;

    bool operator==(const SectionLevel& other) const;
    bool operator!=(const SectionLevel& other) const;

    // Like operator!= but with logLevel argument
    bool diff(const SectionLevel& other, LogLevel logLevel) const;
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
               int32_t lineNumber)
        : _type(type)
        , _sectionId(sectionId)
        , _points(std::move(points))
        , _details(std::move(details))
        , _lineNumber(lineNumber) {}

    AnnotationType _type;
    uint32_t _sectionId;
    PointLevel _points;
    std::string _details;
    int32_t _lineNumber;
};

struct Marker {
    PointLevel _pointLevel;
    std::string _label;
    int32_t _sectionId;  // id of section that contains the marker
};

struct CellLevel {
    MorphologyVersion _version = {"undefined", 0, 0};
    morphio::CellFamily _cellFamily = NEURON;
    SomaType _somaType = SOMA_UNDEFINED;
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
    PointLevel _pointLevel;
    SectionLevel _sectionLevel;
    CellLevel _cellLevel;
    PointLevel _somaLevel;

    MitochondriaPointLevel _mitochondriaPointLevel;
    MitochondriaSectionLevel _mitochondriaSectionLevel;

    EndoplasmicReticulumLevel _endoplasmicReticulumLevel;

    template <typename T>
    std::vector<typename T::Type>& get_mut() noexcept;

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

std::ostream& operator<<(std::ostream& os, const Properties& properties);
std::ostream& operator<<(std::ostream& os, const PointLevel& pointLevel);

#define INSTANTIATE_TEMPLATE_GET(T, M)                                       \
    template <>                                                              \
    inline std::vector<T::Type>& Properties::get_mut<T>() noexcept {         \
        return M;                                                            \
    }                                                                        \
    template <>                                                              \
    inline const std::vector<T::Type>& Properties::get<T>() const noexcept { \
        return M;                                                            \
    }

INSTANTIATE_TEMPLATE_GET(Point, _pointLevel._points)
INSTANTIATE_TEMPLATE_GET(Perimeter, _pointLevel._perimeters)
INSTANTIATE_TEMPLATE_GET(Diameter, _pointLevel._diameters)
INSTANTIATE_TEMPLATE_GET(MitoSection, _mitochondriaSectionLevel._sections)
INSTANTIATE_TEMPLATE_GET(MitoPathLength, _mitochondriaPointLevel._relativePathLengths)
INSTANTIATE_TEMPLATE_GET(MitoNeuriteSectionId, _mitochondriaPointLevel._sectionIds)
INSTANTIATE_TEMPLATE_GET(MitoDiameter, _mitochondriaPointLevel._diameters)
INSTANTIATE_TEMPLATE_GET(Section, _sectionLevel._sections)
INSTANTIATE_TEMPLATE_GET(SectionType, _sectionLevel._sectionTypes)

#undef INSTANTIATE_TEMPLATE_GET

template <>
inline const std::map<int32_t, std::vector<uint32_t>>& Properties::children<Section>() const
    noexcept {
    return _sectionLevel._children;
}

template <>
inline const std::map<int32_t, std::vector<uint32_t>>& Properties::children<MitoSection>() const
    noexcept {
    return _mitochondriaSectionLevel._children;
}

}  // namespace Property
}  // namespace morphio
