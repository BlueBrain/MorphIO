#include <limits>

#include <morphio/errorMessages.h>
#include <morphio/section.h>
#include <morphio/soma.h>
#include <morphio/vector_types.h>

#include "point_utils.h"  // centerOfGravity, euclideanDistance, maxDistanceToCenterOfGravity

namespace morphio {

Soma::Soma(const Property::Properties& properties)
    : soma_type_(properties._cellLevel._somaType)
    , properties_(properties._somaLevel) {}


Soma::Soma(const Property::PointLevel& point_properties)
    : properties_(point_properties) {}


Point Soma::center() const {
    return centerOfGravity(points());
}


floatType Soma::volume() const {

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
        morphio::readers::ErrorMessages err;
        throw SomaError(err.ERROR_NOT_IMPLEMENTED_UNDEFINED_SOMA("Soma::volume"));
    }
}


floatType Soma::surface() const {
    const auto& soma_points = points();
    const auto& soma_diameters = diameters();

    size_t size = soma_points.size();

    switch (soma_type_) {
    case SOMA_SINGLE_POINT:
    case SOMA_NEUROMORPHO_THREE_POINT_CYLINDERS: {
        floatType radius = soma_diameters[0] / 2;
        return 4 * morphio::PI * radius * radius;
    }
    case SOMA_CYLINDERS: {
        // Surface is approximated as the sum of areas of the conical frustums
        // defined by each segments. Does not include the endcaps areas
        floatType surface = 0;
        for (unsigned int i = 0; i < size - 1; ++i) {
            floatType r0 = static_cast<morphio::floatType>(soma_diameters[i]) * floatType{0.5};
            floatType r1 = static_cast<morphio::floatType>(soma_diameters[i + 1]) * floatType{0.5};
            floatType h2 = euclidean_distance(soma_points[i], soma_points[i + 1]);
            auto s = morphio::PI * (r0 + r1) * std::sqrt((r0 - r1) * (r0 - r1) + h2 * h2);
            surface += s;
        }
        return surface;
    }
    case SOMA_SIMPLE_CONTOUR: {
        throw NotImplementedError("Surface is not implemented for SOMA_SIMPLE_CONTOUR");
    }
    case SOMA_UNDEFINED:
    default: {
        morphio::readers::ErrorMessages err;
        throw SomaError(err.ERROR_NOT_IMPLEMENTED_UNDEFINED_SOMA("Soma::surface"));
    }
    }
}


floatType Soma::maxDistance() const {
    return maxDistanceToCenterOfGravity(points());
}
}  // namespace morphio
