#include <morphio/mut/soma.h>
#include <morphio/soma.h>

#include "../shared_utils.hpp"

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
    return _somaSurface<std::vector<morphio::floatType>, std::vector<Point>>(type(),
                                                                             diameters(),
                                                                             points());
}

floatType Soma::maxDistance() const {
    return maxDistanceToCenterOfGravity(point_properties_._points);
}

std::ostream& operator<<(std::ostream& os, const Soma& soma) {
    os << dumpPoints(soma.points());
    return os;
}

std::ostream& operator<<(std::ostream& os, const std::shared_ptr<Soma>& soma) {
    os << *soma;
    return os;
}

}  // end namespace mut
}  // end namespace morphio
