/* Copyright (c) 2013-2023, EPFL/Blue Brain Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include <iosfwd>  // std::ostream
#include <memory>  // std::shared_ptr
#include <vector>  // std::vector

#include <morphio/types.h>
#include <morphio/vasc/properties.h>
#include <morphio/vasc/vasculature.h>

namespace morphio {
namespace vasculature {
/** Vasculature section */
class Section
{
    using SectionId = property::VascSection;
    using PointAttribute = property::Point;

  public:
    Section(const Section& section) = default;

    Section& operator=(const Section& section);

    Section(uint32_t id, const std::shared_ptr<property::Properties>& morphology);

    bool operator==(const Section& section) const;
    bool operator!=(const Section& section) const;
    bool operator<(const Section& other) const;

    /**
       Returns a list of predecessors or parents of the section
    **/
    std::vector<Section> predecessors() const;

    /**
       Returns a list of successors or children of the section
    **/
    std::vector<Section> successors() const;

    /**
       Returns a list of all neighbors of the section
    **/
    std::vector<Section> neighbors() const;

    /** Return the ID of this section. */
    uint32_t id() const noexcept;

    /**
       Euclidian distance between first and last point of the section
    **/
    floatType length() const;

    graph_iterator begin() const;
    graph_iterator end() const;

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
     * Return the morphological type of this section (artery, vein, capillary, ...)
     */
    VascularSectionType type() const;

  protected:
    template <typename Property>
    range<const typename Property::Type> get() const;

    uint32_t id_;
    SectionRange range_;
    std::shared_ptr<property::Properties> properties_;
};

}  // namespace vasculature
}  // namespace morphio

std::ostream& operator<<(std::ostream& os, const morphio::vasculature::Section& section);
