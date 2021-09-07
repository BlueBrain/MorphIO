#pragma once

#include <string>
#include <vector>

#include <morphio/morphology.h>
#include <morphio/types.h>

namespace morphio {

class DendriticSpine: public Morphology
{
  public:
    DendriticSpine(const std::string& source);

    const std::vector<Property::DendriticSpine::PostSynapticDensity>& postSynapticDensity() const
        noexcept;

  protected:
    friend class mut::DendriticSpine;
    DendriticSpine(const Property::Properties& properties);

  private:
    Soma soma() const;
    Mitochondria mitochondria() const;
    const EndoplasmicReticulum endoplasmicReticulum() const;
    const std::vector<Property::Annotation>& annotations() const;
    const std::vector<Property::Marker>& markers() const;
    const SomaType& somaType() const;
};

}  // namespace morphio
