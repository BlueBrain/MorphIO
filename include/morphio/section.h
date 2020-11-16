#pragma once

#include <memory>  // std::shared_ptr

#include <morphio/morphology.h>
#include <morphio/properties.h>
#include <morphio/section_base.h>
#include <morphio/section_iterators.hpp>
#include <morphio/types.h>

namespace morphio {
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

using upstream_iterator = upstream_iterator_t<Section>;
using breadth_iterator = breadth_iterator_t<Section, Morphology>;
using depth_iterator = depth_iterator_t<Section, Morphology>;

class Section: public SectionBase<Section>
{
    using SectionId = Property::Section;
    using PointAttribute = Property::Point;

  public:
    /**
       Depth first search iterator
    **/
    depth_iterator depth_begin() const;
    depth_iterator depth_end() const;

    /**
       Breadth first search iterator
    **/
    breadth_iterator breadth_begin() const;
    breadth_iterator breadth_end() const;

    /**
       Upstream first search iterator
    **/
    upstream_iterator upstream_begin() const;
    upstream_iterator upstream_end() const;

    /**
     * Return a view
    (https://github.com/isocpp/CppCoreGuidelines/blob/master/docs/gsl-intro.md#gslspan-what-is-gslspan-and-what-is-it-for)
     to this section's point coordinates
    **/
    range<const Point> points() const;

    /**
     * Return a view
    (https://github.com/isocpp/CppCoreGuidelines/blob/master/docs/gsl-intro.md#gslspan-what-is-gslspan-and-what-is-it-for)
     to this section's point diameters
    **/
    range<const floatType> diameters() const;

    /**
     * Return a view
     (https://github.com/isocpp/CppCoreGuidelines/blob/master/docs/gsl-intro.md#gslspan-what-is-gslspan-and-what-is-it-for)
     to this section's point perimeters
     **/
    range<const floatType> perimeters() const;

    /**
     * Return the morphological type of this section (dendrite, axon, ...)
     */
    SectionType type() const;
    friend class mut::Section;
    friend Section Morphology::section(uint32_t) const;
    friend class SectionBase<Section>;

  protected:
    Section(uint32_t id_, const std::shared_ptr<Property::Properties>& properties)
        : SectionBase(id_, properties) {}
};

// explicit instanciation
template class SectionBase<Section>;

}  // namespace morphio

std::ostream& operator<<(std::ostream& os, const morphio::Section& section);
std::ostream& operator<<(std::ostream& os, const morphio::range<const morphio::Point>& points);
