/* Copyright (c) 2013-2023, EPFL/Blue Brain Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include <string>
#include <vector>

#include <morphio/mut/morphology.h>  // mut::Morphology
#include <morphio/properties.h>      // Property

namespace morphio {
namespace mut {

/** Mutable(editable) morphio::DendriticSpine */
class DendriticSpine: public Morphology
{
  public:
    DendriticSpine();
    explicit DendriticSpine(const std::string& source);


    /// Returns the post synaptic density values
    std::vector<Property::DendriticSpine::PostSynapticDensity>& postSynapticDensity() noexcept;

    /// Returns the post synaptic density values
    const std::vector<Property::DendriticSpine::PostSynapticDensity>& postSynapticDensity() const
        noexcept;
};

}  // namespace mut
}  // namespace morphio
