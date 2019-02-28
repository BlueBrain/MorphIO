#pragma once

#include <cstdint>
#include <iomanip>
#include <iostream>
#include <memory>
#include <ostream>

#include <functional>

#include <morphio/errorMessages.h>
#include <morphio/exceptions.h>
#include <morphio/mut/iterators.h>
#include <morphio/mut/mitochondria.h>
#include <morphio/mut/soma.h>
#include <morphio/properties.h>
#include <morphio/section.h>
#include <morphio/types.h>

namespace morphio {
namespace mut {
bool _checkDuplicatePoint(std::shared_ptr<Section> parent,
    std::shared_ptr<Section> current);

class Morphology
{
public:
    Morphology()
        : _counter(0)
        , _soma(std::make_shared<Soma>())
        , _cellProperties(std::make_shared<morphio::Property::CellLevel>(
              morphio::Property::CellLevel()))
    {
    }

    /**
       Build a mutable Morphology from an on-disk morphology

       options is the modifier flags to be applied. All flags are defined in
    their enum: morphio::enum::Option and can be composed.

       Example:
           Morphology("neuron.asc", TWO_POINTS_SECTIONS | SOMA_SPHERE);
    **/
    Morphology(const morphio::URI& uri, unsigned int options = NO_MODIFIER);

    /**
       Build a mutable Morphology from a mutable morphology
    **/
    Morphology(const morphio::mut::Morphology& morphology, unsigned int options = NO_MODIFIER);

    /**
       Build a mutable Morphology from a read-only morphology
    **/
    Morphology(const morphio::Morphology& morphology, unsigned int options = NO_MODIFIER);

    virtual ~Morphology();

    /**
    Are considered equal, 2 morphologies with the same:\n
     - root sections
     - section topology
     - cell family
     For each section:
     - same points
     - same diameters
     - same perimeters
     - same type
     Note: the soma types are NOT required to be equal
     **/
    bool operator==(const Morphology& other) const;
    bool operator!=(const Morphology& other) const;

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
     * Return the annotation object
     **/
    const std::vector<Property::Annotation> annotations() const;

    /**
       Get the shared pointer for the given section

       Note: multiple morphologies can share the same Section instances.
    **/
    const std::shared_ptr<Section> section(uint32_t id) const;

    /**
       Depth first iterator starting at a given section id

       If id == -1, the iteration will start at each root section, successively
    **/
    depth_iterator depth_begin() const;
    depth_iterator depth_end() const;

    /**
       Breadth first iterator

       If id == -1, the iteration will be successively performed starting
       at each root section
    **/
    breadth_iterator breadth_begin() const;
    breadth_iterator breadth_end() const;

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
       Append the existing morphio::Section as a root section

       If recursive == true, all descendent will be appended as well
    **/
    std::shared_ptr<Section> appendRootSection(const morphio::Section&,
        bool recursive = false);

    /**
       Append an existing Section as a root section

       If recursive == true, all descendent will be appended as well
    **/
    std::shared_ptr<Section> appendRootSection(std::shared_ptr<Section> section,
        bool recursive = false);

    /**
       Append a root Section
    **/
    std::shared_ptr<Section> appendRootSection(const Property::PointLevel&,
        SectionType sectionType);

    /**
       Iterate on all sections starting at startSection via a depth-first-search
    traversal and call the callback function fun.

       startSection specifies the starting section. if startSection == -1, the
    traversal will done on every neurite.
    **/
    // void traverse(std::function<void(Morphology& morphology, uint32_t
    // sectionId)>,
    //               uint32_t startSection = -1);

    void applyModifiers(unsigned int modifierFlags);

    /**
     * Return the soma type
     **/
    SomaType somaType() { return _soma->type(); }

    /**
     * Return the cell family (neuron or glia)
     **/
    CellFamily& cellFamily() { return _cellProperties->_cellFamily; }

    /**
     * Return the version
     **/
    MorphologyVersion& version() { return _cellProperties->_version; }

    /**
     * Write file to H5, SWC, ASC format depending on filename extension
     **/
    void write(const std::string& filename);

    void addAnnotation(const morphio::Property::Annotation& annotation)
    {
        _annotations.push_back(annotation);
    }

    /**
       Return the data structure used to create read-only morphologies
    **/
    const Property::Properties buildReadOnly() const;

    /**
       Check that the neuron is valid, issue warning and fix unifurcations
     **/
    void sanitize();
    void sanitize(const morphio::plugin::DebugInfo& debugInfo);

private:
    friend class Section;
    friend void modifiers::nrn_order(morphio::mut::Morphology& morpho);
    morphio::plugin::ErrorMessages _err;

    uint32_t _register(std::shared_ptr<Section>);
    bool _compare(const Morphology& other, bool verbose) const;

    uint32_t _counter;
    std::shared_ptr<Soma> _soma;
    std::shared_ptr<morphio::Property::CellLevel> _cellProperties;
    std::vector<std::shared_ptr<Section>> _rootSections;
    std::map<uint32_t, std::shared_ptr<Section>> _sections;
    std::vector<morphio::Property::Annotation> _annotations;
    Mitochondria _mitochondria;

    std::map<uint32_t, uint32_t> _parent;
    std::map<uint32_t, std::vector<std::shared_ptr<Section>>> _children;
};

} // namespace mut
} // namespace morphio
