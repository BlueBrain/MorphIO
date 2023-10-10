/* Copyright (c) 2013-2023, EPFL/Blue Brain Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <morphio/endoplasmic_reticulum.h>
#include <morphio/mut/endoplasmic_reticulum.h>


namespace morphio {
namespace mut {
EndoplasmicReticulum::EndoplasmicReticulum(const std::vector<uint32_t>& section_indices,
                                           const std::vector<morphio::floatType>& volumes,
                                           const std::vector<morphio::floatType>& surface_areas,
                                           const std::vector<uint32_t>& filament_counts) {
    properties_._sectionIndices = section_indices;
    properties_._volumes = volumes;
    properties_._surfaceAreas = surface_areas;
    properties_._filamentCounts = filament_counts;
}

EndoplasmicReticulum::EndoplasmicReticulum(
    const morphio::EndoplasmicReticulum& endoplasmic_reticulum)
    : properties_(endoplasmic_reticulum.properties_->_endoplasmicReticulumLevel) {}

const std::vector<uint32_t>& EndoplasmicReticulum::sectionIndices() const noexcept {
    return properties_._sectionIndices;
}
std::vector<uint32_t>& EndoplasmicReticulum::sectionIndices() noexcept {
    return properties_._sectionIndices;
}

const std::vector<morphio::floatType>& EndoplasmicReticulum::volumes() const noexcept {
    return properties_._volumes;
}

std::vector<morphio::floatType>& EndoplasmicReticulum::volumes() noexcept {
    return properties_._volumes;
}

const std::vector<morphio::floatType>& EndoplasmicReticulum::surfaceAreas() const noexcept {
    return properties_._surfaceAreas;
}

std::vector<morphio::floatType>& EndoplasmicReticulum::surfaceAreas() noexcept {
    return properties_._surfaceAreas;
}

const std::vector<uint32_t>& EndoplasmicReticulum::filamentCounts() const noexcept {
    return properties_._filamentCounts;
}

std::vector<uint32_t>& EndoplasmicReticulum::filamentCounts() noexcept {
    return properties_._filamentCounts;
}

Property::EndoplasmicReticulumLevel EndoplasmicReticulum::buildReadOnly() const noexcept {
    return properties_;
}

}  // namespace mut
}  // namespace morphio
