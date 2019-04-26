#pragma once

#include <morphio/vasc/vasculature.h>
#include <morphio/vasc/properties.h>
#include <morphio/types.h>

namespace morphio
{
namespace vasculature
{
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

    //bool isRoot() const;

    const std::vector<Section> predecessors() const;

    const std::vector<Section> successors() const;

    const std::vector<Section> neighbors() const;

    uint32_t id() const;

    float length() const;

    graph_iterator begin() const;
    graph_iterator end() const;

    const range<const Point> points() const;

    const range<const float> diameters() const;

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
