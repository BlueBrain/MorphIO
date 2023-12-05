/* Copyright (c) 2013-2023, EPFL/Blue Brain Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include <map>
#include <memory>

#include <morphio/mito_section.h>
#include <morphio/properties.h>
#include <morphio/types.h>

#include <morphio/mut/mito_section.h>

namespace morphio {
namespace mut {

using mito_upstream_iterator = morphio::upstream_iterator_t<std::shared_ptr<MitoSection>>;
using mito_breadth_iterator =
    morphio::breadth_iterator_t<std::shared_ptr<MitoSection>, Mitochondria>;
using mito_depth_iterator = morphio::depth_iterator_t<std::shared_ptr<MitoSection>, Mitochondria>;

/** Mutable(editable) morphio::Mitochondria */
class Mitochondria
{
    using MitoSectionP = std::shared_ptr<MitoSection>;

  public:
    Mitochondria() = default;

    /// Get the Section children
    const std::vector<MitoSectionP>& children(const MitoSectionP&) const;

    /**
       Get the shared pointer for the given section

       Note: multiple morphologies can share the same Section instances.
    **/
    const MitoSectionP& section(uint32_t id) const;

    /// Returns the dictionary id -> Section for this tree
    const std::map<uint32_t, MitoSectionP>& sections() const noexcept;

    /**
       Depth first iterator starting at a given section id

       If id == -1, the iteration will start at each root section, successively
    **/
    mito_depth_iterator depth_begin() const;
    mito_depth_iterator depth_begin(const MitoSectionP& section) const;
    mito_depth_iterator depth_end() const;

    /**
       Breadth first iterator

       If id == -1, the iteration will be successively performed starting
       at each root section
    **/
    mito_breadth_iterator breadth_begin() const;
    mito_breadth_iterator breadth_begin(const MitoSectionP& section) const;
    mito_breadth_iterator breadth_end() const;

    /**
       Upstream first iterator

       If id == -1, the iteration will be successively performed starting
       at each root section
    **/
    mito_upstream_iterator upstream_begin() const;
    mito_upstream_iterator upstream_begin(const MitoSectionP& section) const;
    mito_upstream_iterator upstream_end() const;

    /**
     * Return the parent mithochondrial section ID
     **/
    const MitoSectionP& parent(const MitoSectionP& parent) const;

    /**
       Return true if section is a root section
    **/
    bool isRoot(const MitoSectionP& section) const;

    /**
     * Return the list of IDs of all mitochondrial root sections
     * (sections whose parent ID are -1)
     **/
    const std::vector<MitoSectionP>& rootSections() const noexcept;

    /**
       Append a new root MitoSection
    **/
    MitoSectionP appendRootSection(const Property::MitochondriaPointLevel& pointProperties);

    /**
       Append a root MitoSection

       If recursive == true, all descendent mito sections will be appended as
    well
    **/
    MitoSectionP appendRootSection(const morphio::MitoSection&, bool recursive = false);
    MitoSectionP appendRootSection(const MitoSectionP&, bool recursive = false);

    const MitoSectionP& mitoSection(uint32_t id) const;

    /**
       Fill the 'properties' variable with the mitochondria data
    **/
    void _buildMitochondria(Property::Properties& properties) const;

  private:
    friend class MitoSection;

    uint32_t _register(const MitoSectionP& section);

    uint32_t _counter = 0;
    std::map<uint32_t, std::vector<MitoSectionP>> children_;
    std::map<uint32_t, uint32_t> parent_;
    std::vector<MitoSectionP> root_sections_;
    std::map<uint32_t, MitoSectionP> sections_;
};

inline const std::map<uint32_t, Mitochondria::MitoSectionP>& Mitochondria::sections() const
    noexcept {
    return sections_;
}

inline const std::vector<Mitochondria::MitoSectionP>& Mitochondria::rootSections() const noexcept {
    return root_sections_;
}

}  // namespace mut
}  // namespace morphio
