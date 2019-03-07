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
    uint32_t _id;
    SectionRange _range;
    std::shared_ptr<Property::Properties> _properties;

public:
    
    VasculatureSection(const VasculatureSection& section);
    
    bool isRoot() const;
    
    const std::vector<VasculatureSection> predecessors() const;
    
    const std::vector<VasculatureSection> successors() const;
    
    const std::vector<VasculatureSection> neighbors() const;

    const uint32_t id() const;

    const float length() const;

    depth_iterator depth_begin() const;
    depth_iterator depth_end() const;
    
    breadth_iterator breadth_begin() const;
    breadth_iterator breadth_end() const;

    upstream_iterator upstream_begin() const;
    upstream_iterator upstream_end() const;

    const range<const Point> points() const;

    const range<const float> diameters() const;

    const SectionType type() const;    
};

}

std::ostream& operator<<(std::ostream& os, const morphio::VasculatureSection& section);
std::ostream& operator<<(std::ostream& os, morphio::range<const morphio::Point> points);
