#include <morphio/spine.h>

namespace morphio {
Spine::Spine(const std::string& source) : Morphology(source) {
    _properties->_cellLevel._cellFamily = CellFamily::SPINE;
}
}
