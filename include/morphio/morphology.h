#pragma once

#include <memory>  //std::unique_ptr

#include <highfive/H5Group.hpp>
#include <morphio/properties.h>
#include <morphio/section.h>
#include <morphio/section_iterators.hpp>
#include <morphio/ttree.tpp>
#include <morphio/types.h>

namespace morphio {

class Morphology: public TTree<NeuronalSection, Morphology, morphio::mut::Morphology>
{
  public:
    Morphology(const std::string& source, unsigned int options = NO_MODIFIER);
    Morphology(const HighFive::Group& group, unsigned int options = NO_MODIFIER);
    Morphology(morphio::mut::Morphology morphology);

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
    Morphology(const Property::Properties& properties, unsigned int options = NO_MODIFIER);

  private:
    void init();
};

}  // namespace morphio
