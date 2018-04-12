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

template <typename T>
class SectionBase
{
public:
    SectionBase(const SectionBase& section);

    const SectionBase& operator=(const SectionBase& section);

    bool operator==(const SectionBase& section) const;
    bool operator!=(const SectionBase& section) const;

    bool isRoot() const;

    /**
     * Return the parent section of this section \if pybind or None if doesn't
     * have any.\else.
     *
     * @throw runtime_error is the section doesn't have a parent.
     * \endif
     */
    T parent() const;

    /**
     * Return a vector with all the direct children of this section.
     * The container will be empty for terminal sections.
     */
    const std::vector<T> children() const;

    /** Return the ID of this section. */
    const uint32_t id() const;

protected:
    SectionBase(uint32_t id, std::shared_ptr<Property::Properties> morphology);
    template <typename Property> const range<const typename Property::Type> get() const;


    uint32_t _id;
    SectionRange _range;
    std::shared_ptr<Property::Properties> _properties;
};

} // namespace morphio

std::ostream& operator<<(std::ostream& os, const morphio::Section& section);
std::ostream& operator<<(std::ostream& os, morphio::range<const morphio::Point> points);

#include "../../src/section_base.tpp"
