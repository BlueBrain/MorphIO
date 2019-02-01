#pragma once

#include <morphio/iterators.h>
#include <morphio/morphology.h>
#include <morphio/properties.h>
#include <morphio/section_base.h>
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

class Section : public SectionBase<Section>
{
    using SectionId = Property::Section;
    using PointAttribute = Property::Point;

public:
    /**
       Euclidian distance between first and last point of the section
    **/
    float length() const;

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
    const range<const Point> points() const;

    /**
     * Return a view
    (https://github.com/isocpp/CppCoreGuidelines/blob/master/docs/gsl-intro.md#gslspan-what-is-gslspan-and-what-is-it-for)
     to this section's point diameters
    **/
    const range<const float> diameters() const;

    /**
     * Return a view
     (https://github.com/isocpp/CppCoreGuidelines/blob/master/docs/gsl-intro.md#gslspan-what-is-gslspan-and-what-is-it-for)
     to this section's point perimeters
     **/
    const range<const float> perimeters() const;

    /**
     * Return the morphological type of this section (dendrite, axon, ...)
     */
    SectionType type() const;
    friend class mut::Section;
    friend const Section Morphology::section(const uint32_t&) const;
    friend class SectionBase<Section>;

protected:
    Section(uint32_t id, std::shared_ptr<Property::Properties> morphology)
        : SectionBase(id, morphology)
    {
    }
};

// explicit instanciation
template class SectionBase<Section>;

} // namespace morphio

std::ostream& operator<<(std::ostream& os, const morphio::Section& section);
std::ostream& operator<<(std::ostream& os,
    morphio::range<const morphio::Point> points);
