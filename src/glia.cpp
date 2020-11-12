#include <morphio/glia.h>

namespace morphio {
Glia::Glia(const std::string& source) : Morphology(source) {
    _properties->_cellLevel._cellFamily = CellFamily::GLIA;
}
}
