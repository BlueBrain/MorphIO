#pragma once

#include <minimorph/api.h>
#include <minimorph/types.h>
#include <minimorph/morphology.h>
#include <minimorph/properties.h>
#include <minimorph/iterators.h>


namespace minimorph
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
 * minimorph::Morphology and minimorph::Section methods.
 *
 * This is a lightweight object with STL container style thread safety.
 * It is also safe to use a section after the morphology from where it comes
 * has been deallocated. The morphological data will be kept as long as there
 * is a Section referring to it.
 */
class Section
{
public:
    BRAIN_API Section(const Section& section);

    BRAIN_API Section& operator=(const Section& section);

    BRAIN_API bool operator==(const Section& section) const;
    BRAIN_API bool operator!=(const Section& section) const;

    /** Return the ID of this section. */
    BRAIN_API uint32_t getID() const;

    /** Return the morphological type of this section (dendrite, axon, ...). */
    BRAIN_API const SectionType getType() const;



    /**
     * Return the parent section of this section \if pybind or None if doesn't
     * have any.\else.
     *
     * @throw runtime_error is the section doesn't have a parent.
     * \endif
     */
    std::shared_ptr<Section> getParent() const;
        // BRAIN_API std::experimental::optional<Section> getParent() const;

    /**
     * Return a vector with all the direct children of this section.
     * The container will be empty for terminal sections.
     */
    BRAIN_API Sections getChildren() const;

    /**
       Depth first search iterator
    **/
    BRAIN_API depth_iterator depth_begin();
    BRAIN_API depth_iterator depth_end();

    /**
       Breadth first search iterator
    **/
    BRAIN_API breadth_iterator breadth_begin();
    BRAIN_API breadth_iterator breadth_end();

    /**
       Upstream first search iterator
    **/
    BRAIN_API upstream_iterator upstream_begin();
    BRAIN_API upstream_iterator upstream_end();


    const Points getPoints() const;
    const floats getDiameters() const;
    const floats getPerimeters() const;
    const SectionTypes getSectionTypes() const;


private:
    BRAIN_API Section(uint32_t id, PropertiesPtr morphology);
    template <typename Property> const typename Property::Type get() const;

    friend class Morphology;
    friend class Soma;

    uint32_t _id;
    SectionRange _range;
    PropertiesPtr _properties;
};


std::ostream& operator<<(std::ostream& os, const Section& section);

}
