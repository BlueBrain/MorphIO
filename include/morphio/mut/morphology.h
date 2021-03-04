#pragma once

#include <cstdint>
#include <iomanip>
#include <iostream>
#include <memory>
#include <ostream>
#include <unordered_map>

#include <functional>

#include <morphio/errorMessages.h>
#include <morphio/exceptions.h>
#include <morphio/mut/endoplasmic_reticulum.h>
#include <morphio/mut/mitochondria.h>
#include <morphio/mut/soma.h>
#include <morphio/properties.h>
#include <morphio/section.h>
#include <morphio/types.h>

namespace morphio {
namespace mut {
bool _checkDuplicatePoint(const std::shared_ptr<Section>& parent,
                          const std::shared_ptr<Section>& current);

class Morphology
{
  public:
    Morphology()
        : _counter(0)
        , _soma(std::make_shared<Soma>())
        , _cellProperties(
              std::make_shared<morphio::Property::CellLevel>(morphio::Property::CellLevel())) {}

    /**
       Build a mutable Morphology from an on-disk morphology

       options is the modifier flags to be applied. All flags are defined in
    their enum: morphio::enum::Option and can be composed.

       Example:
           Morphology("neuron.asc", TWO_POINTS_SECTIONS | SOMA_SPHERE);
    **/
    Morphology(const std::string& uri, unsigned int options = NO_MODIFIER);

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
       Returns all section ids at the tree root
    **/
    inline const std::vector<std::shared_ptr<Section>>& rootSections() const noexcept;

    /**
       Returns the dictionary id -> Section for this tree
    **/
    inline const std::map<uint32_t, std::shared_ptr<Section>>& sections() const noexcept;

    /**
       Returns a shared pointer on the Soma

       Note: multiple morphologies can share the same Soma instance
    **/
    inline std::shared_ptr<Soma>& soma() noexcept;

    /**
       Returns a shared pointer on the Soma

       Note: multiple morphologies can share the same Soma instance
    **/
    inline const std::shared_ptr<Soma>& soma() const noexcept;

    /**
     * Return the mitochondria container class
     **/
    inline Mitochondria& mitochondria() noexcept;
    /**
     * Return the mitochondria container class
     **/
    inline const Mitochondria& mitochondria() const noexcept;

    /**
     * Return the endoplasmic reticulum container class
     **/
    inline EndoplasmicReticulum& endoplasmicReticulum() noexcept;
    /**
     * Return the endoplasmic reticulum container class
     **/
    inline const EndoplasmicReticulum& endoplasmicReticulum() const noexcept;

    /**
     * Return the annotation object
     **/
    inline const std::vector<Property::Annotation>& annotations() const noexcept;

    /**
     * Return the markers from the ASC file
     **/
    inline const std::vector<Property::Marker>& markers() const noexcept;

    /**
       Get the shared pointer for the given section

       Note: multiple morphologies can share the same Section instances.
    **/
    inline const std::shared_ptr<Section>& section(uint32_t id) const;

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

       Will silently fail if the section is not part of the tree

       If recursive == true, all descendent sections will be deleted as well
       Else, children will be re-attached to their grand-parent
    **/
    void deleteSection(std::shared_ptr<Section> section, bool recursive = true);

    /**
       Append the existing morphio::Section as a root section

       If recursive == true, all descendent will be appended as well
    **/
    std::shared_ptr<Section> appendRootSection(const morphio::Section&, bool recursive = false);

    /**
       Append an existing Section as a root section

       If recursive == true, all descendent will be appended as well
    **/
    std::shared_ptr<Section> appendRootSection(const std::shared_ptr<Section>& section,
                                               bool recursive = false);

    /**
       Append a root Section
    **/
    std::shared_ptr<Section> appendRootSection(const Property::PointLevel&,
                                               SectionType sectionType);

    void applyModifiers(unsigned int modifierFlags);

    /**
     * Return the soma type
     **/
    inline SomaType somaType() const noexcept;

    /**
     * Return the cell family (neuron or glia)
     **/
    inline CellFamily cellFamily() const noexcept;

    /**
     * Return the version
     **/
    inline MorphologyVersion version() const noexcept;

    /**
     * Write file to H5, SWC, ASC format depending on filename extension
     **/
    void write(const std::string& filename);

    inline void addAnnotation(const morphio::Property::Annotation& annotation);
    inline void addMarker(const morphio::Property::Marker& marker);

    /**
       Return the data structure used to create read-only morphologies
    **/
    Property::Properties buildReadOnly() const;

    /**
     * Return the graph connectivity of the morphology where each section
     * is seen as a node
     * Note: -1 is the soma node
     **/
    std::unordered_map<int, std::vector<unsigned int>> connectivity();


    /**
       Fixes the morphology single child sections and issues warnings
       if the section starts and ends are inconsistent
     **/
    void removeUnifurcations();
    void removeUnifurcations(const morphio::readers::DebugInfo& debugInfo);

    /**
       Used before writing to SWC to check that there is no unifurcation
     **/
    void _raiseIfUnifurcations();

  public:
    friend class Section;
    friend void modifiers::nrn_order(morphio::mut::Morphology& morpho);
    friend bool diff(const Morphology& left,
                     const Morphology& right,
                     morphio::enums::LogLevel verbose);
    morphio::readers::ErrorMessages _err;

    uint32_t _register(const std::shared_ptr<Section>&);

    uint32_t _counter;
    std::shared_ptr<Soma> _soma;
    std::shared_ptr<morphio::Property::CellLevel> _cellProperties;
    std::vector<std::shared_ptr<Section>> _rootSections;
    std::map<uint32_t, std::shared_ptr<Section>> _sections;
    Mitochondria _mitochondria;
    EndoplasmicReticulum _endoplasmicReticulum;

    std::map<uint32_t, uint32_t> _parent;
    std::map<uint32_t, std::vector<std::shared_ptr<Section>>> _children;

  private:
    void eraseByValue(std::vector<std::shared_ptr<Section>>& vec,
                      const std::shared_ptr<Section> section);
};

inline const std::vector<std::shared_ptr<Section>>& Morphology::rootSections() const noexcept {
    return _rootSections;
}

inline const std::map<uint32_t, std::shared_ptr<Section>>& Morphology::sections() const noexcept {
    return _sections;
}

inline std::shared_ptr<Soma>& Morphology::soma() noexcept {
    return _soma;
}

inline const std::shared_ptr<Soma>& Morphology::soma() const noexcept {
    return _soma;
}

inline Mitochondria& Morphology::mitochondria() noexcept {
    return _mitochondria;
}

inline const Mitochondria& Morphology::mitochondria() const noexcept {
    return _mitochondria;
}

inline EndoplasmicReticulum& Morphology::endoplasmicReticulum() noexcept {
    return _endoplasmicReticulum;
}

inline const EndoplasmicReticulum& Morphology::endoplasmicReticulum() const noexcept {
    return _endoplasmicReticulum;
}

inline const std::shared_ptr<Section>& Morphology::section(uint32_t id) const {
    return _sections.at(id);
}

inline SomaType Morphology::somaType() const noexcept {
    return _soma->type();
}

inline const std::vector<Property::Annotation>& Morphology::annotations() const noexcept {
    return _cellProperties->_annotations;
}

inline const std::vector<Property::Marker>& Morphology::markers() const noexcept {
    return _cellProperties->_markers;
}

inline CellFamily Morphology::cellFamily() const noexcept {
    return _cellProperties->_cellFamily;
}

inline MorphologyVersion Morphology::version() const noexcept {
    return _cellProperties->_version;
}

inline void Morphology::addAnnotation(const morphio::Property::Annotation& annotation) {
    _cellProperties->_annotations.push_back(annotation);
}

inline void Morphology::addMarker(const morphio::Property::Marker& marker) {
    _cellProperties->_markers.push_back(marker);
}

}  // namespace mut
}  // namespace morphio
