#pragma once

#include <morphio/mito_section.h>
#include <morphio/properties.h>
#include <morphio/types.h>

namespace morphio {
namespace mut {

class MitoSection
{
  public:
    MitoSection(Mitochondria* mitochondria,
                unsigned int id,
                const Property::MitochondriaPointLevel& pointProperties);
    MitoSection(Mitochondria* mitochondria, unsigned int id, const morphio::MitoSection& section);
    MitoSection(Mitochondria* mitochondria, unsigned int id, const MitoSection& section);

    std::shared_ptr<MitoSection> appendSection(const Property::MitochondriaPointLevel& points);

    std::shared_ptr<MitoSection> appendSection(const std::shared_ptr<MitoSection>& original_section,
                                               bool recursive);

    std::shared_ptr<MitoSection> appendSection(const morphio::MitoSection& section, bool recursive);

    std::shared_ptr<MitoSection> parent() const;
    bool isRoot() const;
    const std::vector<std::shared_ptr<MitoSection>>& children() const;

    /**
     * Return the section id
     **/
    inline uint32_t id() const noexcept;

    /** @{
     * Return the diameters of all points of this section
     **/
    inline const std::vector<morphio::floatType>& diameters() const noexcept;
    inline std::vector<morphio::floatType>& diameters() noexcept;
    /** @} */

    /** @{
     * Return the neurite section Ids of all points of this section
     **/
    inline const std::vector<uint32_t>& neuriteSectionIds() const noexcept;
    inline std::vector<uint32_t>& neuriteSectionIds() noexcept;
    /** @} */

    /** @{
     * Return the relative distance (between 0 and 1)
     * between the start of the neuronal section and each point
     * of this mitochondrial section
     **/
    inline const std::vector<morphio::floatType>& pathLengths() const noexcept;
    inline std::vector<morphio::floatType>& pathLengths() noexcept;
    /** @} */

  private:
    uint32_t _id;

    Mitochondria* _mitochondria;

  public:
    // TODO: make private
    Property::MitochondriaPointLevel _mitoPoints;
};

inline uint32_t MitoSection::id() const noexcept {
    return _id;
}

inline const std::vector<morphio::floatType>& MitoSection::diameters() const noexcept {
    return _mitoPoints._diameters;
}

inline const std::vector<uint32_t>& MitoSection::neuriteSectionIds() const noexcept {
    return _mitoPoints._sectionIds;
}

inline const std::vector<morphio::floatType>& MitoSection::pathLengths() const noexcept {
    return _mitoPoints._relativePathLengths;
}

inline std::vector<morphio::floatType>& MitoSection::diameters() noexcept {
    return _mitoPoints._diameters;
}

inline std::vector<uint32_t>& MitoSection::neuriteSectionIds() noexcept {
    return _mitoPoints._sectionIds;
}

inline std::vector<morphio::floatType>& MitoSection::pathLengths() noexcept {
    return _mitoPoints._relativePathLengths;
}

void friendDtorForSharedPtrMito(MitoSection* section);

}  // namespace mut
}  // namespace morphio
