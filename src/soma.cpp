#include <limits>

#include <morphio/section.h>
#include <morphio/soma.h>
#include <morphio/vector_types.h>

#include "point_utils.h"
#include "shared_utils.hpp"

namespace morphio {

const floatType NaN = std::numeric_limits<floatType>::quiet_NaN();

Soma::Soma(const Property::Properties& properties)
    : soma_type_(properties._cellLevel._somaType)
    , properties_(properties._somaLevel) {}


Soma::Soma(const Property::PointLevel& point_properties)
    : properties_(point_properties) {}


Point Soma::center() const {

    if (points().empty()) {
        return {NaN, NaN, NaN};
    }
    return centerOfGravity(points());
}


floatType Soma::volume() const {

    if (diameters().empty()) {
        return NaN;
    }

    switch (soma_type_) {
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
    if (points().empty()) {
        return NaN;
    }
    return _somaSurface(type(), diameters(), points());
}


floatType Soma::maxDistance() const {
    if (points().empty()) {
        return NaN;
    }
    return maxDistanceToCenterOfGravity(points());
}
}  // namespace morphio
