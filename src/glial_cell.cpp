/* Copyright (c) 2013-2023, EPFL/Blue Brain Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <morphio/exceptions.h>  // for RawDataError
#include <morphio/glial_cell.h>

namespace morphio {

GlialCell::GlialCell(const std::string& source)
    : Morphology(source) {
    if (properties_->_cellLevel._cellFamily != CellFamily::GLIA) {
        throw(RawDataError("File: " + source +
                           " is not a GlialCell file. It should be a H5 file the cell type GLIA."));
    }
}

}  // namespace morphio
