#pragma once

#include <memory>  // std::shared_ptr

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

class Morphology;

template <typename Family>
class Node: public SectionBase<Node<Family>>
{
    using SectionId = Property::Section;
    using PointAttribute = Property::Point;

  public:
    using Type = typename Family::Type;

    /**
       Depth first search iterator
    **/
    depth_iterator_t<Node> depth_begin() const;
    depth_iterator_t<Node> depth_end() const;

    /**
       Breadth first search iterator
    **/
    breadth_iterator_t<Node> breadth_begin() const;
    breadth_iterator_t<Node> breadth_end() const;

    /**
       Upstream first search iterator
    **/
    upstream_iterator_t<Node> upstream_begin() const;
    upstream_iterator_t<Node> upstream_end() const;

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
    typename Family::Type type() const;
    friend class mut::Section;
    friend class mut::GlialSection;

    template<typename Node, typename CRTP, typename Mut>
    friend class TTree;

    friend class SectionBase<Node>;

  protected:
    Node<Family>(uint32_t id_, const std::shared_ptr<Property::Properties>& properties)
        : SectionBase<Node<Family>>(id_, properties) {}
};

// explicit instanciation
extern template class Node<CellFamily::NEURON>;
extern template class Node<CellFamily::GLIA>;

}  // namespace morphio

template <typename Family>
std::ostream& operator<<(std::ostream& os, const morphio::Node<Family>& section);
std::ostream& operator<<(std::ostream& os, const morphio::range<const morphio::Point>& points);
