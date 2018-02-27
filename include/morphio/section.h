#pragma once


#include <morphio/iterators.h>
#include <morphio/morphology.h>
#include <morphio/properties.h>
#include <morphio/types.h>

namespace morphio
{
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
class Section
{
public:
    Section(const Section& section);

    const Section& operator=(const Section& section);

    bool operator==(const Section& section) const;
    bool operator!=(const Section& section) const;

    bool isRoot() const;

    /**
     * Return the parent section of this section \if pybind or None if doesn't
     * have any.\else.
     *
     * @throw runtime_error is the section doesn't have a parent.
     * \endif
     */
    Section parent() const;

    /**
     * Return a vector with all the direct children of this section.
     * The container will be empty for terminal sections.
     */
    const std::vector<Section> children() const;

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

    const range<const Point> points() const;
    const range<const float> diameters() const;
    const range<const float> perimeters() const;

    /** Return the morphological type of this section (dendrite, axon, ...). */
    const SectionType type() const;

    /** Return the ID of this section. */
    const uint32_t id() const;

private:
    Section(uint32_t id, std::shared_ptr<Property::Properties> morphology);
    template <typename Property> const range<const typename Property::Type> get() const;

    friend class mut::Section;
    friend const Section Morphology::section(const uint32_t&) const;

    uint32_t _id;
    SectionRange _range;
    std::shared_ptr<Property::Properties> _properties;
};


}

std::ostream& operator<<(std::ostream& os, const morphio::Section& section);
std::ostream& operator<<(std::ostream& os, morphio::range<const morphio::Point> points);
