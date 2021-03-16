#pragma once

#include <functional>

#include <morphio/properties.h>
#include <morphio/section.h>
#include <morphio/types.h>

#include <morphio/section_iterators.hpp>

namespace morphio {
namespace mut {


using glial_upstream_iterator = morphio::upstream_iterator_t<std::shared_ptr<GlialSection>>;
using glial_breadth_iterator = morphio::breadth_iterator_t<std::shared_ptr<GlialSection>>;
using glial_depth_iterator = morphio::depth_iterator_t<std::shared_ptr<GlialSection>>;

class GlialSection: public std::enable_shared_from_this<GlialSection>
{
  public:
    ~GlialSection() = default;

    /**
     * Return the section ID
     **/
    inline uint32_t id() const noexcept;

    /** @{
     * Return the morphological type of this section (dendrite, axon, ...)
     **/
    inline GlialSectionType& type() noexcept;
    inline const GlialSectionType& type() const noexcept;
    /** @} */

    /** @{
       Return the coordinates (x,y,z) of all points of this section
    **/
    inline std::vector<Point>& points() noexcept;
    inline const std::vector<Point>& points() const noexcept;
    /** @} */

    /** @{
       Return the diameters of all points of this section
    **/
    inline std::vector<morphio::floatType>& diameters() noexcept;
    inline const std::vector<morphio::floatType>& diameters() const noexcept;
    /** @} */

    /** @{
       Return the perimeters of all points of this section
    **/
    inline std::vector<morphio::floatType>& perimeters() noexcept;
    inline const std::vector<morphio::floatType>& perimeters() const noexcept;
    /** @} */

    /** @{
       Return the PointLevel instance that contains this section's data
    **/
    inline Property::PointLevel& properties() noexcept;
    inline const Property::PointLevel& properties() const noexcept;
    /** @} */
    ////////////////////////////////////////////////////////////////////////////////
    //
    // Methods that were previously in mut::GlialCell
    //
    ////////////////////////////////////////////////////////////////////////////////

    /**
       Get the parent ID

       Note: Root sections return -1
    **/
    const std::shared_ptr<GlialSection>& parent() const;

    /**
       Return true if section is a root section
    **/
    bool isRoot() const;

    /**
       Return a vector of children IDs
    **/
    const std::vector<std::shared_ptr<GlialSection>>& children() const;

    glial_depth_iterator depth_begin() const;
    glial_depth_iterator depth_end() const;

    glial_breadth_iterator breadth_begin() const;
    glial_breadth_iterator breadth_end() const;

    glial_upstream_iterator upstream_begin() const;
    glial_upstream_iterator upstream_end() const;


    std::shared_ptr<GlialSection> appendSection(const morphio::Node<CellFamily::GLIA>&, bool recursive = false);

    std::shared_ptr<GlialSection> appendSection(const std::shared_ptr<GlialSection>& original_section,
                                           bool recursive = false);

    std::shared_ptr<GlialSection> appendSection(
        const Property::PointLevel&, GlialSectionType sectionType = GlialSectionType::UNDEFINED);

  private:
    friend class GlialCell;


    GlialSection(GlialCell*, unsigned int id, GlialSectionType type, const Property::PointLevel&);
    GlialSection(GlialCell*, unsigned int id, const morphio::Node<CellFamily::GLIA>& section);
    GlialSection(GlialCell*, unsigned int id, const GlialSection&);

    GlialCell* _morphology;
    Property::PointLevel _pointProperties;
    uint32_t _id;
    GlialSectionType _sectionType;
};

std::ostream& operator<<(std::ostream&, const std::shared_ptr<GlialSection>&);

inline uint32_t GlialSection::id() const noexcept {
    return _id;
}

inline GlialSectionType& GlialSection::type() noexcept {
    return _sectionType;
}

inline const GlialSectionType& GlialSection::type() const noexcept {
    return _sectionType;
}

inline std::vector<Point>& GlialSection::points() noexcept {
    return _pointProperties._points;
}

inline const std::vector<Point>& GlialSection::points() const noexcept {
    return _pointProperties._points;
}

inline std::vector<morphio::floatType>& GlialSection::diameters() noexcept {
    return _pointProperties._diameters;
}

inline const std::vector<morphio::floatType>& GlialSection::diameters() const noexcept {
    return _pointProperties._diameters;
}

inline std::vector<morphio::floatType>& GlialSection::perimeters() noexcept {
    return _pointProperties._perimeters;
}

inline const std::vector<morphio::floatType>& GlialSection::perimeters() const noexcept {
    return _pointProperties._perimeters;
}

inline Property::PointLevel& GlialSection::properties() noexcept {
    return _pointProperties;
}

inline const Property::PointLevel& GlialSection::properties() const noexcept {
    return _pointProperties;
}

}  // namespace mut
}  // namespace morphio

std::ostream& operator<<(std::ostream&, const morphio::mut::GlialSection&);
