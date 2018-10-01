#pragma once

#include <cstdint>
#include <iomanip>
#include <iostream>
#include <memory>
#include <ostream>

#include <functional>

#include <morphio/errorMessages.h>
#include <morphio/exceptions.h>
#include <morphio/mut/mitochondria.h>
#include <morphio/properties.h>
#include <morphio/section.h>
#include <morphio/types.h>
#include <morphio/mut/soma.h>
#include <morphio/mut/iterators.h>


namespace morphio
{
namespace mut
{

class Morphology
{
public:
    Morphology() : _soma(std::make_shared<Soma>(Soma())),
                   _cellProperties(std::make_shared<morphio::Property::CellLevel>(morphio::Property::CellLevel())),
                   _counter(0) {}

    /**
       Build a mutable Morphology from an on-disk morphology

       options is the modifier flags to be applied. All flags are defined in their enum:
       morphio::enum::Option and can be composed.

       Example:
           Morphology("neuron.asc", TWO_POINTS_SECTIONS | SOMA_SPHERE);
    **/
    Morphology(const morphio::URI& uri, unsigned int options = NO_MODIFIER);

    /**
       Build a mutable Morphology from a read-only morphology
    **/
    Morphology(const morphio::Morphology& morphology);

    virtual ~Morphology();


    /**
       Returns all section ids at the tree root
    **/
    const std::vector<std::shared_ptr<Section>>& rootSections() const;


    /**
       Returns the dictionary id -> Section for this tree
    **/
    const std::map<uint32_t, std::shared_ptr<Section>> sections() const;

    /**
       Returns a shared pointer on the Soma

       Note: multiple morphologies can share the same Soma instance
    **/
    std::shared_ptr<Soma> soma();
    const std::shared_ptr<Soma> soma() const;

    /**
     * Return the mitochondria container class
     **/
    Mitochondria& mitochondria() { return _mitochondria; }
    const Mitochondria& mitochondria() const { return _mitochondria; }


    /**
       Get the shared pointer for the given section

       Note: multiple morphologies can share the same Section instances.
    **/
    const std::shared_ptr<Section> section(uint32_t id) const;


    /**
       Return the data structure used to create read-only morphologies
    **/
    const Property::Properties buildReadOnly() const;



    /**
       Depth first iterator starting at a given section id

       If id == -1, the iteration will start at each root section, successively
    **/
    depth_iterator depth_begin() const;
    depth_iterator depth_begin(const std::shared_ptr<Section>& section) const;
    depth_iterator depth_end() const;

    /**
       Breadth first iterator

       If id == -1, the iteration will be successively performed starting
       at each root section
    **/
    breadth_iterator breadth_begin() const;
    breadth_iterator breadth_begin(const std::shared_ptr<Section>& section) const;
    breadth_iterator breadth_end() const;

    /**
       Upstream first iterator
    **/
    upstream_iterator upstream_begin(const std::shared_ptr<Section>& section) const;
    upstream_iterator upstream_end() const;


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
    void deleteSection(std::shared_ptr<Section> section, bool recursive = true);

    /**
       Append the existing morphio::Section to the given parentId (nullptr appends to soma)

       If recursive == true, all descendent will be appended as well
    **/
    std::shared_ptr<Section> appendSection(std::shared_ptr<Section> parent,
                                           const morphio::Section&,
                                           bool recursive = false);

    /**
       Append the existing Section to the given parentId (nullptr appends to soma)

       If a mut::morphio::Morphology is passed, all descendent of section in this
       morphology will be appended as well
    **/
    std::shared_ptr<Section> appendSection(std::shared_ptr<Section> parent,
                                           std::shared_ptr<Section> section,
                                           const Morphology& morphology = Morphology());


    /**
       Append a new Section the given parentId (nullptr appends to soma)

       If sectionType is omitted or set to SECTION_UNDEFINED,
       the type of the parent section will be used
       (Root sections can't have sectionType ommited)
    **/
    std::shared_ptr<Section> appendSection(std::shared_ptr<Section> parent,
                                           const Property::PointLevel&,
                                           SectionType sectionType = SectionType::SECTION_UNDEFINED);

    /**
       Iterate on all sections starting at startSection via a depth-first-search traversal
       and call the callback function fun.

       startSection specifies the starting section. if startSection == -1, the traversal
       will done on every neurite.
    **/
    // void traverse(std::function<void(Morphology& morphology, uint32_t sectionId)>,
    //               uint32_t startSection = -1);

    void applyModifiers(unsigned int modifierFlags);

    /**
     * Return the soma type
     **/
    SomaType somaType() { return _soma->type(); }

    /**
     * Return the cell family (neuron or glia)
     **/
    CellFamily& cellFamily(){ return _cellProperties->_cellFamily; }

    /**
     * Return the version
     **/
    MorphologyVersion& version(){ return _cellProperties->_version; }

    /**
     * Write file to H5, SWC, ASC format depending on filename extension
     **/
    void write(const std::string& filename);

    void addAnnotation(const morphio::Property::Annotation& annotation) {
        _annotations.push_back(annotation);
    }

    const Property::Properties buildReadOnly(const morphio::plugin::DebugInfo& debugInfo) const;

protected:
    /**
     * Write file to H5 format
     **/
    virtual void _write_h5(const std::string& filename);

    /**
     * Write file to ASC (neurolucida) format
     **/
    virtual void _write_asc(const std::string& filename);

    /**
     * Write file to SWC format
     **/
    virtual void _write_swc(const std::string& filename);


private:
    friend class Section;
    friend void modifiers::nrn_order(morphio::mut::Morphology& morpho);
    morphio::plugin::ErrorMessages _err;

    uint32_t _register(std::shared_ptr<Section>&);
    std::shared_ptr<Soma> _soma;
    std::vector<std::shared_ptr<Section>> _rootSections;
    std::map<uint32_t, std::shared_ptr<Section>> _sections;
    std::shared_ptr<morphio::Property::CellLevel> _cellProperties;
    std::vector<morphio::Property::Annotation> _annotations;
    Mitochondria _mitochondria;

    uint32_t _counter;
    std::map<uint32_t, uint32_t> _parent;
    std::map<uint32_t, std::vector<std::shared_ptr<Section>>> _children;

};

} // namespace mut
} // namespace morphio
