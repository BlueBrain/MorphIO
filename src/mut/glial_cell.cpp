/* Copyright (c) 2013-2023, EPFL/Blue Brain Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <morphio/mut/glial_cell.h>

namespace morphio {
namespace mut {

GlialCell::GlialCell()
    : Morphology() {
    _cellProperties->_cellFamily = CellFamily::GLIA;
}

GlialCell::GlialCell(const std::string& source)
    : Morphology(source) {
    if (_cellProperties->_cellFamily != CellFamily::GLIA) {
        throw(RawDataError("File: " + source +
                           " is not a GlialCell file. It should be a H5 file the cell type GLIA."));
    }
}

}  // namespace mut
}  // namespace morphio
