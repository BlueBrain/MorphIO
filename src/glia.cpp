#include <morphio/glia.h>

namespace morphio {

Glia::Glia(const std::string& source)
    : Morphology(source) {
    if (_properties->_cellLevel._cellFamily != CellFamily::GLIA)
        throw(RawDataError("File: " + source +
                           " is not a Glia file. It should be a H5 file the cell type GLIA."));
}

}  // namespace morphio
