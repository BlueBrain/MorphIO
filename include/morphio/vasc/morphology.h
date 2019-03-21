#pragma once

#include <memory>
#include <morphio/vasc/properties.h>
#include <morphio/types.h>

namespace morphio {
namespace vasculature {
class VasculatureMorphology
{
public:
    virtual ~VasculatureMorphology();

    VasculatureMorphology& operator=(const VasculatureMorphology&);
    VasculatureMorphology(VasculatureMorphology&&);
    VasculatureMorphology& operator=(VasculatureMorphology&&);

    bool operator==(const VasculatureMorphology& other) const;
    bool operator!=(const VasculatureMorphology& other) const;

    VasculatureMorphology(const URI& source, unsigned int options = NO_MODIFIER);

    //const std::vector<VasculatureSection> rootSections() const;

    const std::vector<VasculatureSection> sections() const;

    const VasculatureSection section(const uint32_t& id) const;

    const Points& points() const;

    const std::vector<float>& diameters() const;

    const std::vector<property::SectionType::Type>& sectionTypes() const;

    graph_iterator begin() const;
    graph_iterator end() const;

private:

    std::shared_ptr<property::Properties> _properties;

    template <typename Property>
    const std::vector<typename Property::Type>& get() const;

};
} // namespace vasculature
} // namespace morphio