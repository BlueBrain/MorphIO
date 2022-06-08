#pragma once

#include <memory>  // std::shared_ptr

#include <morphio/morphology.h>
#include <morphio/properties.h>
#include <morphio/section_base.h>
#include <morphio/section_iterators.hpp>
#include <morphio/types.h>

namespace morphio {

using upstream_iterator = upstream_iterator_t<Section>;
using breadth_iterator = breadth_iterator_t<Section, Morphology>;
using depth_iterator = depth_iterator_t<Section, Morphology>;

/**
 * A class to represent a morphological section.
 *
 * A Section is an unbranched piece of a morphological skeleton.
 * This class provides functions to query information about the sample points
 * that compose the section and functions to obtain the parent and children
 * sections.
 *
 * The cell soma is also considered a section, but some functions have
 * special meaning for it.
 *
 * Sections cannot be directly created, but are returned by several
 * morphio::Morphology and morphio::Section methods.
 *
 * This is a lightweight object with STL container style thread safety.
 * It is also safe to use a section after the morphology from where it comes
 * has been deallocated. The morphological data will be kept as long as there
 * is a Section referring to it.
 */
class Section: public SectionBase<Section>
{
    using SectionId = Property::Section;
    using PointAttribute = Property::Point;

  public:
    /// Depth first iterator
    depth_iterator depth_begin() const {
        return depth_iterator(*this);
    }
    depth_iterator depth_end() const {
        return depth_iterator();
    }

    /// Breadth first iterator
    breadth_iterator breadth_begin() const {
        return breadth_iterator(*this);
    }
    breadth_iterator breadth_end() const {
        return breadth_iterator();
    }

    /// Upstream iterator
    upstream_iterator upstream_begin() const {
        return upstream_iterator(*this);
    }
    upstream_iterator upstream_end() const {
        return upstream_iterator();
    }

    /**
     * Return a view
    (https://github.com/isocpp/CppCoreGuidelines/blob/master/docs/gsl-intro.md#gslspan-what-is-gslspan-and-what-is-it-for)
     to this section's point coordinates
    **/
    range<const Point> points() const {
        return get<Property::Point>();
    }

    /**
     * Return a view
    (https://github.com/isocpp/CppCoreGuidelines/blob/master/docs/gsl-intro.md#gslspan-what-is-gslspan-and-what-is-it-for)
     to this section's point diameters
    **/
    range<const floatType> diameters() const {
        return get<Property::Diameter>();
    }

    /**
     * Return a view
     (https://github.com/isocpp/CppCoreGuidelines/blob/master/docs/gsl-intro.md#gslspan-what-is-gslspan-and-what-is-it-for)
     to this section's point perimeters
     **/
    range<const floatType> perimeters() const {
        return get<Property::Perimeter>();
    }

    /// Return the morphological type of this section (dendrite, axon, ...)
    SectionType type() const {
        return properties_->get<Property::SectionType>()[id_];
    }

    /**
     * Return true if the sections of the tree downstream (downstream = true) or upstream
     * (donwstream = false) have the same section type as the current section.
     */
    bool isHeterogeneous(bool downstream = true) const;

    /// Return true if the both sections have the same points, diameters and perimeters
    bool hasSameShape(const Section& other) const noexcept;

    friend class mut::Section;
    friend Section Morphology::section(uint32_t) const;
    friend class SectionBase<Section>;

  protected:
    Section(uint32_t id, const std::shared_ptr<Property::Properties>& properties)
        : SectionBase(id, properties) {}
};

}  // namespace morphio

std::ostream& operator<<(std::ostream& os, const morphio::Section& section);
