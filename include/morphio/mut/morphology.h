#pragma once

#include <cstdint>
#include <iomanip>
#include <iostream>
#include <memory>
#include <ostream>

#include <functional>


#include <morphio/exceptions.h>
#include <morphio/iterators.h>
#include <morphio/properties.h>
#include <morphio/section.h>
#include <morphio/types.h>
#include <morphio/mut/soma.h>

namespace morphio
{
namespace mut
{


class Morphology
{
public:
    Morphology() : _soma(std::make_shared<Soma>(Soma())) , _counter(0) {}

    /**
       Build a mutable Morphology from a read-only morphology
    **/
    Morphology(const morphio::Morphology& morphology);

    virtual ~Morphology();


    /**
       Returns all section ids at the tree root
    **/
    const std::set<uint32_t>& rootSections();


    /**
       Returns the dictionary id -> Section for this tree
    **/
    const std::map<uint32_t, std::shared_ptr<Section>>& sections() const;

    /**
       Returns a shared pointer on the Soma

       Note: multiple morphologies can share the same Soma instance
    **/
    std::shared_ptr<Soma>& soma();

    /**
       Get the shared pointer for the given section identified

       Note: multiple morphologies can shared the same shared pointers.
    **/
    const std::shared_ptr<Section> section(uint32_t id);

    /**
       Get the parent ID
    **/
    const uint32_t parent(uint32_t id);

    /**
       Get children IDs
    **/
    const std::set<uint32_t> children(uint32_t id);


    /**
       Return the data structure used to create read-only morphologies
    **/
    const Property::Properties buildReadOnly();

    ////////////////////////////////////////////////////////////////////////////////
    //
    //   Tree manipulation methods
    //
    ////////////////////////////////////////////////////////////////////////////////

    /**
       Delete the given section

       Will silently fail if the section id is not part of the tree

       If recursive == true, all descendent sections will be deleted as well
       Else, children will be re-attached to their grand-parent
    **/
    void deleteSection(uint32_t id, bool recursive = true);

    /**
       Append the read-only Section to the given parentId

       If recursive == true, all descendent will be appended as well
    **/
    uint32_t appendSection(uint32_t parentId, const morphio::Section&, bool recursive = true);

    /**
       Append the read-only Section to the given parentId

       If recursive == true, all descendent will be appended as well
    **/
    uint32_t appendSection(uint32_t parentId, SectionType, const Property::PointLevel&);

    /**
       Iterate on all sections starting at startSection via a depth-first-search traversal
       and call the callback function fun.

       startSection specifies the starting section. if startSection == -1, the traversal
       will done on every neurite.
    **/
    void traverse(std::function<void(Morphology& morphology, uint32_t sectionId)>,
                  uint32_t startSection = -1);



private:
    friend class Section;

    uint32_t _register(std::shared_ptr<Section>);
    std::shared_ptr<Soma> _soma;
    std::set<uint32_t> _rootSections;
    std::map<uint32_t, std::shared_ptr<Section>> _sections;

    uint32_t _counter;
    std::map<uint32_t, uint32_t> _parent;
    std::map<uint32_t, std::set<uint32_t>> _children;
};

} // namespace mut
} // namespace morphio
