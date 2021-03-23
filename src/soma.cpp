#include <morphio/section.h>
#include <morphio/soma.h>
#include <morphio/vector_types.h>

#include "shared_utils.hpp"

namespace morphio {
Soma::Soma(const std::shared_ptr<Property::Properties>& properties)
    : _properties(properties) {}

Point Soma::center() const {
    return centerOfGravity(_properties->_somaLevel._points);
}

floatType Soma::volume() const {
    switch (_properties->_cellLevel._somaType) {
    case SOMA_NEUROMORPHO_THREE_POINT_CYLINDERS: {
        floatType radius = diameters()[0] / 2;
        return 4 * morphio::PI * radius * radius;
    }

    case SOMA_SINGLE_POINT:
    case SOMA_CYLINDERS:
    case SOMA_SIMPLE_CONTOUR:
    case SOMA_UNDEFINED:
    default:
        throw;
    }
}

floatType Soma::surface() const {
    return _somaSurface<range<const floatType>, range<const Point>>(type(), diameters(), points());
}

floatType Soma::maxDistance() const {
    return maxDistanceToCenterOfGravity(_properties->_somaLevel._points);
}

}  // namespace morphio
