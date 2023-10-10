/* Copyright (c) 2013-2023, EPFL/Blue Brain Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include <string>
#include <vector>

#include <morphio/enums.h>       // SomaType
#include <morphio/morphology.h>  // Morphology
#include <morphio/types.h>       // EndoplasmicReticulum, Mitochondria

namespace morphio {

/** Class to represent morphologies of dendritic spines */
class DendriticSpine: public Morphology
{
  public:
    explicit DendriticSpine(const std::string& source);

    const std::vector<Property::DendriticSpine::PostSynapticDensity>& postSynapticDensity() const
        noexcept;

  protected:
    friend class mut::DendriticSpine;
    explicit DendriticSpine(const Property::Properties& properties);

  private:
    Soma soma() const;
    Mitochondria mitochondria() const;
    const EndoplasmicReticulum endoplasmicReticulum() const;
    const std::vector<Property::Annotation>& annotations() const;
    const std::vector<Property::Marker>& markers() const;
    const SomaType& somaType() const;
};

}  // namespace morphio
