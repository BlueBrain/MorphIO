/* Copyright (c) 2013-2023, EPFL/Blue Brain Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <morphio/mut/soma.h>
#include <morphio/soma.h>

#include "../point_utils.h"     // centerOfGRavity, maxDistanceToCenterOfGravity
#include "../shared_utils.hpp"  // _somaSurface

namespace morphio {
namespace mut {
Soma::Soma(const Property::PointLevel& point_properties)
    : point_properties_(point_properties) {}

Soma::Soma(const morphio::Soma& soma)
    : soma_type_(soma.type())
    , point_properties_(soma.properties_->_somaLevel) {}

Point Soma::center() const {
    return centerOfGravity(points());
}

floatType Soma::surface() const {
    return _somaSurface(type(), diameters(), points());
}

floatType Soma::maxDistance() const {
    return maxDistanceToCenterOfGravity(point_properties_._points);
}

}  // end namespace mut
}  // end namespace morphio
