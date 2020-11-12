#include <morphio/mut/glia.h>

namespace morphio {
namespace mut {

Glia::Glia() : Morphology() {
    _cellProperties->_cellFamily = CellFamily::GLIA;
}

Glia::Glia(const std::string& source) : Morphology(source) {
    if (_cellProperties->_cellFamily != CellFamily::GLIA)
        throw(RawDataError("File: " + source +
                           " is not a Glia file. It should be a H5 file the cell type GLIA."));
}

}  // namespace mut
}  // namespace morphio
