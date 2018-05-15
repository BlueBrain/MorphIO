#pragma once

#include <morphio/properties.h>
#include <morphio/types.h>

namespace morphio
{
namespace mut
{
class MitoSection
{
public:
    MitoSection(const Property::MitochondriaPointLevel& mitoPoints)
        : _mitoPoints(mitoPoints)
    {
    }

    /**
     * Return the diameters of all points of this section
     **/
    std::vector<float>& diameters() { return _mitoPoints._diameters; }

    /**
     * Return the neurite section Ids of all points of this section
     **/
    std::vector<uint32_t>& neuriteSectionIds() { return _mitoPoints._sectionIds; }

    /**
     * Return the relative distance (between 0 and 1)
     * between the start of the neuronal section and each point
     * of this mitochondrial section
     **/
    std::vector<float>& pathLengths() { return _mitoPoints._relativePathLengths; }

    Property::MitochondriaPointLevel _mitoPoints;
};

/**
 * The entry-point class to access mitochondrial data
 *
 * By design, it is the equivalent of the Morphology class but at the mitochondrial level.
 * As the Morphology class, it implements a section accessor and a root section accessor
 * returning views on the Properties object for the queried mitochondrial section.
 **/
class Mitochondria
{
public:
    Mitochondria() :_mitochondriaSectionCounter(0)
    {
    }

    const std::vector<uint32_t> children(uint32_t id) const;
    const std::shared_ptr<MitoSection> section(uint32_t id) const;
    const std::map<uint32_t, std::shared_ptr<MitoSection>>& sections() const;

    /**
       Depth first iterator starting at a given section id

       If id == -1, the iteration will start at each root section, successively
    **/
    mito_depth_iterator depth_begin(uint32_t id = -1) const;
    mito_depth_iterator depth_end() const;

    /**
       Breadth first iterator

       If id == -1, the iteration will be successively performed starting
       at each root section
    **/
    mito_breadth_iterator breadth_begin(uint32_t id = -1) const;
    mito_breadth_iterator breadth_end() const;

    /**
       Upstream first iterator

       If id == -1, the iteration will be successively performed starting
       at each root section
    **/
    mito_upstream_iterator upstream_begin(uint32_t id = -1) const;
    mito_upstream_iterator upstream_end() const;



    /**
     * Return the parent mithochondrial section ID
     **/
    const int32_t parent(uint32_t id) const;

    /**
       Return true if section is a root section
    **/
    const bool isRoot(uint32_t id) const;


    /**
     * Return the list of IDs of all mitochondrial root sections
     * (sections whose parent ID are -1)
     **/
    const std::vector<uint32_t>& rootSections() const;


    uint32_t appendSection(int32_t mitoParentId,
                           const Property::MitochondriaPointLevel& points);

    const std::shared_ptr<MitoSection> mitoSection(uint32_t id) const;

    void _buildMitochondria(Property::Properties& properties) const;

private:
    std::map<uint32_t, std::vector<uint32_t>> _children;
    std::map<uint32_t, uint32_t> _parent;
    std::vector<uint32_t> _rootSections;
    std::map<uint32_t, std::shared_ptr<MitoSection>> _sections;

    uint32_t _mitochondriaSectionCounter;
};
}
}
