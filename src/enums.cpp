#include <morphio/enums.h>
#include <ostream>

namespace morphio {
namespace enums {

/** Output stream formatter for MorphologyVersion */
std::ostream& operator<<(std::ostream& os, const SomaType v) {
    switch (v) {
    case SOMA_SINGLE_POINT:
        return os << "SOMA_SINGLE_POINT";
    case SOMA_NEUROMORPHO_THREE_POINT_CYLINDERS:
        return os << "SOMA_NEUROMORPHO_THREE_POINT_CYLINDERS";
    case SOMA_CYLINDERS:
        return os << "SOMA_CYLINDERS";
    case SOMA_SIMPLE_CONTOUR:
        return os << "SOMA_SIMPLE_CONTOUR";

    default:
    case SOMA_UNDEFINED:
        return os << "SOMA_UNDEFINED";
    }
}

}  // namespace enums
}  // namespace morphio
