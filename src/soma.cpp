/* Copyright (c) 2013-2023, EPFL/Blue Brain Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <morphio/section.h>
#include <morphio/soma.h>
#include <morphio/vector_types.h>
#include <stdexcept>

#include "point_utils.h"
#include "shared_utils.hpp"

namespace morphio {
Soma::Soma(const std::shared_ptr<Property::Properties>& properties)
    : properties_(properties) {}

Point Soma::center() const {
    return centerOfGravity(properties_->_somaLevel._points);
}

floatType Soma::volume() const {
    switch (properties_->_cellLevel._somaType) {
    case SOMA_NEUROMORPHO_THREE_POINT_CYLINDERS: {
        floatType radius = diameters()[0] / 2;
        return 4 * morphio::PI * radius * radius;
    }

    case SOMA_SINGLE_POINT:
    case SOMA_CYLINDERS:
    case SOMA_SIMPLE_CONTOUR:
    case SOMA_UNDEFINED:
    default:
        throw std::runtime_error("Volume is not supported");
    }
}

floatType Soma::surface() const {
    return _somaSurface(type(), diameters(), points());
}

floatType Soma::maxDistance() const {
    return maxDistanceToCenterOfGravity(properties_->_somaLevel._points);
}

}  // namespace morphio
