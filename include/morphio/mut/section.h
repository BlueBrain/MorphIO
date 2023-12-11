/* Copyright (c) 2013-2023, EPFL/Blue Brain Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include <morphio/properties.h>
#include <morphio/section.h>
#include <morphio/types.h>

#include <morphio/section_iterators.hpp>

namespace morphio {
namespace mut {

using upstream_iterator = morphio::upstream_iterator_t<std::shared_ptr<Section>>;
using breadth_iterator = morphio::breadth_iterator_t<std::shared_ptr<Section>, Morphology>;
using depth_iterator = morphio::depth_iterator_t<std::shared_ptr<Section>, Morphology>;

/** Mutable(editable) morphio::Section */
class Section: public std::enable_shared_from_this<Section>
{
  public:
    ~Section() = default;

    /**
     * Return the section ID
     **/
    inline uint32_t id() const noexcept;

    /** @{
     * Return the morphological type of this section (dendrite, axon, ...)
     **/
    inline SectionType& type() noexcept;
    inline const SectionType& type() const noexcept;
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
    // Methods that were previously in mut::Morphology
    //
    ////////////////////////////////////////////////////////////////////////////////

    /**
       Get the parent ID

       Note: Root sections return -1
    **/
    const std::shared_ptr<Section>& parent() const;

    /**
       Return true if section is a root section
    **/
    bool isRoot() const;

    /**
     * Return true if the sections of the tree downstream (downstream = true) or upstream
     * (donwstream = false) have the same section type as the current section.
     */
    bool isHeterogeneous(bool downstream = true) const;

    /**
     * Return true if the both sections have the same points, diameters and perimeters
     */
    bool hasSameShape(const Section& other) const noexcept;

    /**
       Return a vector of children IDs
    **/
    const std::vector<std::shared_ptr<Section>>& children() const;

    depth_iterator depth_begin() const;
    depth_iterator depth_end() const;

    breadth_iterator breadth_begin() const;
    breadth_iterator breadth_end() const;

    upstream_iterator upstream_begin() const;
    upstream_iterator upstream_end() const;

    std::shared_ptr<Section> appendSection(const morphio::Section&, bool recursive = false);

    std::shared_ptr<Section> appendSection(std::shared_ptr<Section> original_section,
                                           bool recursive = false);

    std::shared_ptr<Section> appendSection(
        const Property::PointLevel&, SectionType sectionType = SectionType::SECTION_UNDEFINED);

  private:
    friend class Morphology;

    Section(Morphology*, unsigned int id, SectionType type, const Property::PointLevel&);
    Section(Morphology*, unsigned int id, const morphio::Section& section);
    Section(Morphology*, unsigned int id, const Section&);


    /**
      If section is an orphan, in other words it does not belong to an a morphology
    **/
    void throwIfNoOwningMorphology() const;

    /**
      Getter for morphology_; checks the pointer is non-null, throws otherwise
    **/
    Morphology* getOwningMorphologyOrThrow() const;

    Morphology* morphology_;
    Property::PointLevel point_properties_;
    uint32_t id_;
    SectionType section_type_;
};

std::ostream& operator<<(std::ostream&, const std::shared_ptr<Section>&);

inline uint32_t Section::id() const noexcept {
    return id_;
}

inline SectionType& Section::type() noexcept {
    return section_type_;
}

inline const SectionType& Section::type() const noexcept {
    return section_type_;
}

inline std::vector<Point>& Section::points() noexcept {
    return point_properties_._points;
}

inline const std::vector<Point>& Section::points() const noexcept {
    return point_properties_._points;
}

inline std::vector<morphio::floatType>& Section::diameters() noexcept {
    return point_properties_._diameters;
}

inline const std::vector<morphio::floatType>& Section::diameters() const noexcept {
    return point_properties_._diameters;
}

inline std::vector<morphio::floatType>& Section::perimeters() noexcept {
    return point_properties_._perimeters;
}

inline const std::vector<morphio::floatType>& Section::perimeters() const noexcept {
    return point_properties_._perimeters;
}

inline Property::PointLevel& Section::properties() noexcept {
    return point_properties_;
}

inline const Property::PointLevel& Section::properties() const noexcept {
    return point_properties_;
}

}  // namespace mut
}  // namespace morphio

std::ostream& operator<<(std::ostream&, const morphio::mut::Section&);
