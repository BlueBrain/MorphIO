#pragma once

#include <memory>  //std::unique_ptr

#include <highfive/H5Group.hpp>
#include <morphio/properties.h>
#include <morphio/section_iterators.hpp>
#include <morphio/section.h>
#include <morphio/types.h>
#include <morphio/ttree.tpp>
#include <morphio/mut/glial_cell.h>

namespace morphio {

// extern template class TTree<morphio::Section, morphio::GlialCell, morphio::mut::GlialCell>;

// extern template class TTree<SectionType>;
// extern template class TTree<GlialSectionType>;


class GlialCell: public TTree<Node<CellFamily::GLIA>, GlialCell, morphio::mut::GlialCell> {
public:
    GlialCell(const std::string& source, unsigned int options = NO_MODIFIER);
    GlialCell(const HighFive::Group& group, unsigned int options = NO_MODIFIER);
    GlialCell(morphio::mut::GlialCell glialCell);

    /**
     * Return the soma object
     **/
    Soma soma() const;

    /**
     * Return the mitochondria object
     **/
    Mitochondria mitochondria() const;

    /**
     * Return the endoplasmic reticulum object
     **/
    const EndoplasmicReticulum endoplasmicReticulum() const;

    /**
     * Return the annotation object
     **/
    const std::vector<Property::Annotation>& annotations() const;

    /**
     * Return the markers
     **/
    const std::vector<Property::Marker>& markers() const;

    /**
     * Return the soma type
     **/
    const SomaType& somaType() const;

protected:
    GlialCell(const Property::Properties& properties, unsigned int options = NO_MODIFIER);

private:
    void init();
};

}  // namespace morphio
