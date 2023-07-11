/* Copyright (c) 2013-2023, EPFL/Blue Brain Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include <cstdint>  // uint32_t
#include <memory>   // std::shared_ptr

#include <morphio/mitochondria.h>
#include <morphio/properties.h>
#include <morphio/section_base.h>
#include <morphio/types.h>

namespace morphio {
using mito_upstream_iterator = upstream_iterator_t<MitoSection>;
using mito_breadth_iterator = morphio::breadth_iterator_t<MitoSection, Mitochondria>;
using mito_depth_iterator = morphio::depth_iterator_t<MitoSection, Mitochondria>;

/** Mitochondria section */
class MitoSection: public SectionBase<MitoSection>
{
    using SectionId = Property::MitoSection;
    using PointAttribute = Property::MitoDiameter;

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

    /**
     * Returns list of neuronal section IDs associated to each point
     **/
    range<const uint32_t> neuriteSectionIds() const;

    /**
     * Returns list of section's point diameters
     **/
    range<const floatType> diameters() const;

    /**
     * Returns list of relative distances between the start
     * of the neuronal section and each point of the mitochondrial section\n
     * Note: - a relative distance of 0 means the mitochondrial point is at the
     *         beginning of the neuronal section
     *       - a relative distance of 1 means the mitochondrial point is at the
     *         end of the neuronal section
     **/
    range<const floatType> relativePathLengths() const;

    /**
     * Return true if the both sections have the same neuriteSectionIds, diameters and relativePathLengths
     */
    bool hasSameShape(const MitoSection& other) const noexcept;

  protected:
    MitoSection(uint32_t id, const std::shared_ptr<Property::Properties>& morphology)
        : SectionBase(id, morphology) {}
    friend MitoSection Mitochondria::section(uint32_t) const;
    friend class SectionBase<MitoSection>;
    friend class mut::MitoSection;
};
}  // namespace morphio
