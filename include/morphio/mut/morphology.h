/* Copyright (c) 2013-2023, EPFL/Blue Brain Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include <morphio/errorMessages.h>
#include <morphio/exceptions.h>
#include <morphio/mut/endoplasmic_reticulum.h>
#include <morphio/mut/mitochondria.h>
#include <morphio/mut/modifiers.h>
#include <morphio/mut/soma.h>
#include <morphio/properties.h>
#include <morphio/types.h>

namespace morphio {
namespace mut {
// TODO: not sure why this is here
bool _checkDuplicatePoint(const std::shared_ptr<Section>& parent,
                          const std::shared_ptr<Section>& current);

/** Mutable(editable) morphio::Morphology */
class Morphology
{
  public:
    Morphology()
        : _soma(std::make_shared<Soma>())
        , _cellProperties(
              std::make_shared<morphio::Property::CellLevel>(morphio::Property::CellLevel())) {}

    /**
       Build a mutable Morphology from an on-disk morphology

       options is the modifier flags to be applied. All flags are defined in
    their enum: morphio::enum::Option and can be composed.

       Example:
           Morphology("neuron.asc", TWO_POINTS_SECTIONS | SOMA_SPHERE);
    **/
    explicit Morphology(const std::string& uri, unsigned int options = NO_MODIFIER);

    /// Build a mutable Morphology from an HighFive::Group
    explicit Morphology(const HighFive::Group& group, unsigned int options = NO_MODIFIER);

    /// Build a mutable Morphology from a mutable morphology
    Morphology(const morphio::mut::Morphology& morphology, unsigned int options = NO_MODIFIER);

    /// Build a mutable Morphology from a read-only morphology
    explicit Morphology(const morphio::Morphology& morphology, unsigned int options = NO_MODIFIER);

    virtual ~Morphology();

    /// Returns all section ids at the tree root
    const std::vector<std::shared_ptr<Section>>& rootSections() const noexcept {
        return _rootSections;
    }

    /// Returns the dictionary id -> Section for this tree
    const std::map<uint32_t, std::shared_ptr<Section>>& sections() const noexcept {
        return _sections;
    }

    /**
       Returns a shared pointer on the Soma

       Note: multiple morphologies can share the same Soma instance
    **/
    std::shared_ptr<Soma>& soma() noexcept {
        return _soma;
    }

    /**
       Returns a shared pointer on the Soma

       Note: multiple morphologies can share the same Soma instance
    **/
    const std::shared_ptr<Soma>& soma() const noexcept {
        return _soma;
    }

    /// Return the mitochondria container class
    Mitochondria& mitochondria() noexcept {
        return _mitochondria;
    }

    /// Return the mitochondria container class
    const Mitochondria& mitochondria() const noexcept {
        return _mitochondria;
    }

    /// Return the endoplasmic reticulum container class
    EndoplasmicReticulum& endoplasmicReticulum() noexcept {
        return _endoplasmicReticulum;
    }
    ///
    /// Return the endoplasmic reticulum container class
    const EndoplasmicReticulum& endoplasmicReticulum() const noexcept {
        return _endoplasmicReticulum;
    }

    /// Return the annotation objects
    const std::vector<Property::Annotation>& annotations() const noexcept {
        return _cellProperties->_annotations;
    }

    /// Return the markers from the ASC file
    const std::vector<Property::Marker>& markers() const noexcept {
        return _cellProperties->_markers;
    }

    /**
       Get the shared pointer for the given section

       Note: multiple morphologies can share the same Section instances.
    **/
    const std::shared_ptr<Section>& section(uint32_t id) const {
        return _sections.at(id);
    }

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

    /// Append a root Section
    std::shared_ptr<Section> appendRootSection(const Property::PointLevel&,
                                               SectionType sectionType);

    void applyModifiers(unsigned int modifierFlags);

    /// Return the soma type
    SomaType somaType() const noexcept {
        return _soma->type();
    }

    /// Return the cell family (neuron or glia)
    CellFamily cellFamily() const noexcept {
        return _cellProperties->_cellFamily;
    }

    /// Return the version
    MorphologyVersion version() const noexcept {
        return _cellProperties->_version;
    }

    /// Write file to H5, SWC, ASC format depending on filename extension
    void write(const std::string& filename) const;

    void addAnnotation(const morphio::Property::Annotation& annotation) {
        _cellProperties->_annotations.push_back(annotation);
    }

    void addMarker(const morphio::Property::Marker& marker) {
        _cellProperties->_markers.push_back(marker);
    }

    /// Return the data structure used to create read-only morphologies
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

    std::shared_ptr<Soma> _soma;
    std::shared_ptr<morphio::Property::CellLevel> _cellProperties;
    EndoplasmicReticulum _endoplasmicReticulum;
    morphio::Property::DendriticSpine::Level _dendriticSpineLevel;

  private:
    std::vector<std::shared_ptr<Section>> _rootSections;
    std::map<uint32_t, std::shared_ptr<Section>> _sections;

    Mitochondria _mitochondria;

    std::map<uint32_t, uint32_t> _parent;
    std::map<uint32_t, std::vector<std::shared_ptr<Section>>> _children;

    uint32_t _counter = 0;

    uint32_t _register(const std::shared_ptr<Section>&);
    morphio::readers::ErrorMessages _err;

    void eraseByValue(std::vector<std::shared_ptr<Section>>& vec,
                      const std::shared_ptr<Section> section);

    friend class Section;
    friend void modifiers::nrn_order(morphio::mut::Morphology& morpho);
    friend bool diff(const Morphology& left,
                     const Morphology& right,
                     morphio::enums::LogLevel verbose);
};

}  // namespace mut
}  // namespace morphio
