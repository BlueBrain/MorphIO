#pragma once

#include <memory>
#include <morphio/vascProperties.h>
#include <morphio/types.h>

namespace morphio
{

class VasculatureMorphology
{
public:
    virtual ~VasculatureMorphology();

    VasculatureMorphology& operator=(const VasculatureMorphology&);
    VasculatureMorphology(VasculatureMorphology&&);
    VasculatureMorphology& operator=(VasculatureMorphology&&);

    explicit VasculatureMorphology(const URI& source);
    
    const std::vector<VasculatureSection> rootSections() const;

    const std::vector<VasculatureSection> sections() const;

    const VasculatureSection section(const uint32_t& id) const;

    const Points& points() const;

    const std::vector<float>& diameters() const;

    const std::vector<SectionType>& sectionTypes() const;

    depth_iterator depth_begin() const;
    depth_iterator depth_end() const;

    breadth_iterator breadth_begin() const;
    breadth_iterator breadth_end() const;

private:
    
    std::shared_ptr<VasculatureProperty::Properties> _ properties;

    template <typename Property>
    const std::vector<typename Property::Type>& get() const;

}

}
