#pragma once

#include <morphio/section_base.h>
#include <morphio/properties.h>
#include <morphio/mitochondria.h>
#include <morphio/types.h>

namespace morphio
{
class MitoSection : public SectionBase<MitoSection>
{
    typedef Property::MitoSection SectionId;
    typedef Property::MitoDiameter PointAttribute;
public:
    /**
       Depth first search iterator
    **/
    mito_depth_iterator depth_begin() const;
    mito_depth_iterator depth_end() const;

    /**
       Breadth first search iterator
    **/
    mito_breadth_iterator breadth_begin() const;
    mito_breadth_iterator breadth_end() const;

    /**
       Upstream first search iterator
    **/
    mito_upstream_iterator upstream_begin() const;
    mito_upstream_iterator upstream_end() const;


    const range<const uint32_t> neuriteSectionId() const;
    const range<const float> diameters() const;
    const range<const float> relativePathLengths() const;

    /** Return the morphological type of this section (dendrite, axon, ...). */
    const SectionType type() const;
    friend const MitoSection Mitochondria::section(const uint32_t&) const;
    friend class SectionBase<MitoSection>;


protected:
    MitoSection(uint32_t id, std::shared_ptr<Property::Properties> morphology) : SectionBase(id, morphology) {}
};
} // namespace morphio
