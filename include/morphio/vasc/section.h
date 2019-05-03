#pragma once

#include <morphio/types.h>
#include <morphio/vasc/properties.h>
#include <morphio/vasc/vasculature.h>

namespace morphio {
namespace vasculature {
class Section
{
    using SectionId = property::VascSection;
    using PointAttribute = property::Point;

public:
    Section(const Section& section);

    const Section& operator=(const Section& section);

    Section(uint32_t id, std::shared_ptr<property::Properties> morphology);

    bool operator==(const Section& section) const;
    bool operator!=(const Section& section) const;
    bool operator<(const Section& other) const;

    /**
       Returns a list of predecessors or parents of the section
    **/
    const std::vector<Section> predecessors() const;

    /**
       Returns a list of successors or children of the section
    **/
    const std::vector<Section> successors() const;

    /**
       Returns a list of all neighbors of the section
    **/
    const std::vector<Section> neighbors() const;

    /** Return the ID of this section. */
    uint32_t id() const;

    /**
       Euclidian distance between first and last point of the section
    **/
    float length() const;

    graph_iterator begin() const;
    graph_iterator end() const;

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
     * Return the morphological type of this section (artery, vein, capillary, ...)
     */
    VascularSectionType type() const;

protected:
    template <typename Property>
    const range<const typename Property::Type> get() const;

    uint32_t _id;
    SectionRange _range;
    std::shared_ptr<property::Properties> _properties;
};
}
}
std::ostream& operator<<(std::ostream& os, const morphio::vasculature::Section& section);
std::ostream& operator<<(std::ostream& os, morphio::range<const morphio::Point> points);
