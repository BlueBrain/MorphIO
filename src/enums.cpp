#include <morphio/enums.h>
#include <ostream>

namespace morphio {
namespace enums {

/** Output stream formatter for MorphologyVersion */
std::ostream& operator<<(std::ostream& os, const MorphologyVersion v) {
    switch (v) {
    case MORPHOLOGY_VERSION_H5_1:
        return os << "h5v1";
    case MORPHOLOGY_VERSION_H5_1_1:
        return os << "h5v1.1";
    case MORPHOLOGY_VERSION_H5_1_2:
        return os << "h5v1.2";
    case MORPHOLOGY_VERSION_H5_2:
        return os << "h5v2";
    case MORPHOLOGY_VERSION_SWC_1:
        return os << "swcv1";
    case MORPHOLOGY_VERSION_ASC_1:
        return os << "ascv1";
    default:
    case MORPHOLOGY_VERSION_UNDEFINED:
        return os << "UNDEFINED";
    }
}

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
