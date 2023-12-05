/* Copyright (c) 2013-2023, EPFL/Blue Brain Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <morphio/endoplasmic_reticulum.h>

namespace morphio {
const std::vector<uint32_t>& EndoplasmicReticulum::sectionIndices() const {
    return properties_->_endoplasmicReticulumLevel._sectionIndices;
}

const std::vector<morphio::floatType>& EndoplasmicReticulum::volumes() const {
    return properties_->_endoplasmicReticulumLevel._volumes;
}

const std::vector<morphio::floatType>& EndoplasmicReticulum::surfaceAreas() const {
    return properties_->_endoplasmicReticulumLevel._surfaceAreas;
}

const std::vector<uint32_t>& EndoplasmicReticulum::filamentCounts() const {
    return properties_->_endoplasmicReticulumLevel._filamentCounts;
}

}  // namespace morphio
