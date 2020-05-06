#include <cmath>

#include <morphio/section.h>
#include <morphio/shared_utils.tpp>
#include <morphio/soma.h>
#include <morphio/vector_types.h>

namespace morphio {
Soma::Soma(const std::shared_ptr<Property::Properties>& properties)
    : _properties(properties) {}

Point Soma::center() const {
    return centerOfGravity(_properties->_somaLevel._points);
}

float Soma::volume() const {
    switch (_properties->_cellLevel._somaType) {
    case SOMA_NEUROMORPHO_THREE_POINT_CYLINDERS: {
        float radius = diameters()[0] / 2;
        return 4 * static_cast<float>(M_PI) * radius * radius;
    }

    case SOMA_SINGLE_POINT:
    case SOMA_CYLINDERS:
    case SOMA_SIMPLE_CONTOUR:
    case SOMA_UNDEFINED:
    default:
        throw;
    }
}

float Soma::surface() const {
    return _somaSurface<range<const float>, range<const Point>>(type(), diameters(), points());
}

float Soma::maxDistance() const {
    return maxDistanceToCenterOfGravity(_properties->_somaLevel._points);
}

}  // namespace morphio
