#pragma once

#include <morphio/mito_section.h>
#include <morphio/properties.h>
#include <morphio/types.h>

#include <morphio/mut/mito_section.h>

namespace morphio {
namespace mut {
/**
 * The entry-point class to access mitochondrial data
 *
 * By design, it is the equivalent of the Morphology class but at the
 *mitochondrial level. As the Morphology class, it implements a section accessor
 *and a root section accessor returning views on the Properties object for the
 *queried mitochondrial section.
 **/
class Mitochondria
{
public:
    Mitochondria()
        : _counter(0)
    {
    }

    const std::vector<std::shared_ptr<MitoSection>> children(
        std::shared_ptr<MitoSection>) const;
    const std::shared_ptr<MitoSection> section(uint32_t id) const;
    const std::map<uint32_t, std::shared_ptr<MitoSection>> sections() const;

    /**
       Depth first iterator starting at a given section id

       If id == -1, the iteration will start at each root section, successively
    **/
    mito_depth_iterator depth_begin() const;
    mito_depth_iterator depth_begin(
        const std::shared_ptr<MitoSection> section) const;
    mito_depth_iterator depth_end() const;

    /**
       Breadth first iterator

       If id == -1, the iteration will be successively performed starting
       at each root section
    **/
    mito_breadth_iterator breadth_begin() const;
    mito_breadth_iterator breadth_begin(
        const std::shared_ptr<MitoSection> section) const;
    mito_breadth_iterator breadth_end() const;

    /**
       Upstream first iterator

       If id == -1, the iteration will be successively performed starting
       at each root section
    **/
    mito_upstream_iterator upstream_begin() const;
    mito_upstream_iterator upstream_begin(
        const std::shared_ptr<MitoSection> section) const;
    mito_upstream_iterator upstream_end() const;

    /**
     * Return the parent mithochondrial section ID
     **/
    const std::shared_ptr<MitoSection> parent(
        const std::shared_ptr<MitoSection> parent) const;

    /**
       Return true if section is a root section
    **/
    bool isRoot(const std::shared_ptr<MitoSection> section) const;

    /**
     * Return the list of IDs of all mitochondrial root sections
     * (sections whose parent ID are -1)
     **/
    const std::vector<std::shared_ptr<MitoSection>>& rootSections() const;

    /**
       Append a new root MitoSection
    **/
    std::shared_ptr<MitoSection> appendRootSection(
        const Property::MitochondriaPointLevel& points);

    /**
       Append a root MitoSection

       If recursive == true, all descendent mito sections will be appended as
    well
    **/
    std::shared_ptr<MitoSection> appendRootSection(const morphio::MitoSection&,
        bool recursive = false);
    std::shared_ptr<MitoSection> appendRootSection(
        const std::shared_ptr<MitoSection>, bool recursive = false);

    const std::shared_ptr<MitoSection> mitoSection(uint32_t id) const;

    void _buildMitochondria(Property::Properties& properties) const;

private:
    friend class MitoSection;

    uint32_t _register(std::shared_ptr<MitoSection> section);

    uint32_t _counter;
    std::map<uint32_t, std::vector<std::shared_ptr<MitoSection>>> _children;
    std::map<uint32_t, uint32_t> _parent;
    std::vector<std::shared_ptr<MitoSection>> _rootSections;
    std::map<uint32_t, std::shared_ptr<MitoSection>> _sections;
};
} // namespace mut
} // namespace morphio
