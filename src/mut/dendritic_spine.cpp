/* Copyright (c) 2013-2023, EPFL/Blue Brain Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <morphio/mut/dendritic_spine.h>

namespace morphio {
namespace mut {

DendriticSpine::DendriticSpine()
    : Morphology() {
    _cellProperties->_cellFamily = CellFamily::SPINE;
    _cellProperties->_version = {"h5", 1, 3};
}

DendriticSpine::DendriticSpine(const std::string& source)
    : Morphology(source) {
    if (_cellProperties->_cellFamily != CellFamily::SPINE) {
        throw(RawDataError(
            "File: " + source +
            " is not a DendriticSpine file. It should be a H5 file the cell type SPINE."));
    }
}

std::vector<Property::DendriticSpine::PostSynapticDensity>&
DendriticSpine::postSynapticDensity() noexcept {
    return _dendriticSpineLevel._post_synaptic_density;
}

const std::vector<Property::DendriticSpine::PostSynapticDensity>&
DendriticSpine::postSynapticDensity() const noexcept {
    return _dendriticSpineLevel._post_synaptic_density;
}

}  // namespace mut
}  // namespace morphio
