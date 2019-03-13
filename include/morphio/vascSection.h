#pragma once

#include <morphio/section_base.h>
#include <morphio/iterators.h>
#include <morphio/vascMorphology.h>
#include <morphio/vascProperties.h>
#include <morphio/types.h>

namespace morphio
{

class VasculatureSection
{
    using SectionId = VasculatureProperty::VascSection;
    using PointAttribute = VasculatureProperty::Point;

public:
    
    VasculatureSection(const VasculatureSection& section);

    const VasculatureSection& operator=(const VasculatureSection& section);

    VasculatureSection(uint32_t id, std::shared_ptr<VasculatureProperty::Properties> morphology);

    bool operator==(const VasculatureSection& section) const;
    bool operator!=(const VasculatureSection& section) const;
    bool operator<(const VasculatureSection& other) const;
    
    //bool isRoot() const;
    
    const std::vector<VasculatureSection> predecessors() const;
    
    const std::vector<VasculatureSection> successors() const;
    
    const std::vector<VasculatureSection> neighbors() const;

    uint32_t id() const;

    float length() const;

    graph_iterator begin() const;
    graph_iterator end() const;

    const range<const Point> points() const;

    const range<const float> diameters() const;

    SectionType type() const;

protected:


    template <typename Property>
    const range<const typename Property::Type> get() const;

    uint32_t _id;
    SectionRange _range;
    std::shared_ptr<VasculatureProperty::Properties> _properties;
};

}

std::ostream& operator<<(std::ostream& os, const morphio::VasculatureSection& section);
std::ostream& operator<<(std::ostream& os, morphio::range<const morphio::Point> points);
