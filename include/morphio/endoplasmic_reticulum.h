/* Copyright (c) 2013-2023, EPFL/Blue Brain Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include <morphio/types.h>  // floatType

#include <cstdint>  // uint32_t
#include <memory>   // std::shared_ptr
#include <utility>  // std::move
#include <vector>   // std::vector

#include <morphio/properties.h>  // Property

namespace morphio {
/**
 * The entry-point class to access endoplasmic reticulum data
 *
 * Spec https://bbpteam.epfl.ch/documentation/projects/Morphology%20Documentation/latest/h5v1.html
 **/
class EndoplasmicReticulum
{
  public:
    /**
       Returns the list of neuronal section indices
    **/
    const std::vector<uint32_t>& sectionIndices() const;

    /**
       Returns the volumes for each neuronal section
    **/
    const std::vector<morphio::floatType>& volumes() const;

    /**
       Returns the surface areas for each neuronal section
    **/
    const std::vector<morphio::floatType>& surfaceAreas() const;

    /**
       Returns the number of filaments for each neuronal section
    **/
    const std::vector<uint32_t>& filamentCounts() const;

  private:
    explicit EndoplasmicReticulum(std::shared_ptr<Property::Properties> properties)
        : properties_(std::move(properties)) {}
    std::shared_ptr<Property::Properties> properties_;

    friend class Morphology;
    friend class morphio::mut::EndoplasmicReticulum;
};
}  // namespace morphio
