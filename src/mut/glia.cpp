#include <morphio/mut/glia.h>

namespace morphio {
namespace mut {

Glia::Glia() : Morphology() {
    _cellProperties->_cellFamily = CellFamily::GLIA;
}

Glia::Glia(const std::string& source) : Morphology(source) {
    _cellProperties->_cellFamily = CellFamily::GLIA;
}

}  // namespace mut
}  // namespace morphio
