#pragma once

#include <morphio/mito_section.h>
#include <morphio/properties.h>
#include <morphio/types.h>

namespace morphio {
namespace mut {
class MitoSection
{
public:
    MitoSection(Mitochondria* mitochondria, int id,
        const Property::MitochondriaPointLevel& pointProperties);
    MitoSection(Mitochondria* mitochondria, int id,
        const morphio::MitoSection& section);
    MitoSection(Mitochondria* mitochondria, int id, const MitoSection& section);

    std::shared_ptr<MitoSection> appendSection(
        const Property::MitochondriaPointLevel& points);

    std::shared_ptr<MitoSection> appendSection(
        std::shared_ptr<MitoSection> original_section, bool recursive);

    std::shared_ptr<MitoSection> appendSection(
        const morphio::MitoSection& section, bool recursive);

    const std::shared_ptr<MitoSection> parent() const;
    bool isRoot() const;
    const std::vector<std::shared_ptr<MitoSection>> children() const;

    /**
     * Return the diameters of all points of this section
     **/
    uint32_t id() const { return _id; }

    /**
     * Return the diameters of all points of this section
     **/
    std::vector<float>& diameters() { return _mitoPoints._diameters; }

    /**
     * Return the neurite section Ids of all points of this section
     **/
    std::vector<uint32_t>& neuriteSectionIds()
    {
        return _mitoPoints._sectionIds;
    }

    /**
     * Return the relative distance (between 0 and 1)
     * between the start of the neuronal section and each point
     * of this mitochondrial section
     **/
    std::vector<float>& pathLengths()
    {
        return _mitoPoints._relativePathLengths;
    }

private:
    friend void friendDtorForSharedPtr(Section*);
    uint32_t _id;

    Mitochondria* _mitochondria;

public:
    // TODO: make private
    Property::MitochondriaPointLevel _mitoPoints;
};

void friendDtorForSharedPtrMito(MitoSection* section);

} // namespace mut
} // namespace morphio
