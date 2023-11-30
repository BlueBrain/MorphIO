/* Copyright (c) 2013-2023, EPFL/Blue Brain Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <morphio/exceptions.h>  // RawDataError
#include <morphio/properties.h>  // Property

#include <morphio/dendritic_spine.h>

namespace morphio {

DendriticSpine::DendriticSpine(const std::string& source)
    : Morphology(source) {
    if (properties_->_cellLevel._cellFamily != CellFamily::SPINE) {
        throw(RawDataError("File: " + source +
                           " is not a DendriticSpine file. It should be a H5 file of type SPINE."));
    }
}

const std::vector<Property::DendriticSpine::PostSynapticDensity>&
DendriticSpine::postSynapticDensity() const noexcept {
    return properties_->_dendriticSpineLevel._post_synaptic_density;
}

}  // namespace morphio
