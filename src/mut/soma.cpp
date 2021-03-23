#include <morphio/mut/soma.h>
#include <morphio/soma.h>

#include <morphio/section.h>

#include "../shared_utils.hpp"

namespace morphio {
namespace mut {
Soma::Soma(const Property::PointLevel& pointProperties)
    : _somaType(SOMA_UNDEFINED)
    , _pointProperties(pointProperties) {}

Soma::Soma(const Soma& soma)
    : _somaType(soma._somaType)
    , _pointProperties(soma._pointProperties) {}

Soma::Soma(const morphio::Soma& soma)
    : _somaType(soma.type())
    , _pointProperties(soma._properties->_somaLevel) {}

Point Soma::center() const {
    return centerOfGravity(points());
}

floatType Soma::surface() const {
    return _somaSurface<std::vector<morphio::floatType>, std::vector<Point>>(type(),
                                                                             diameters(),
                                                                             points());
}

floatType Soma::maxDistance() const {
    return maxDistanceToCenterOfGravity(_pointProperties._points);
}

std::ostream& operator<<(std::ostream& os, const Soma& soma) {
    os << dumpPoints(soma.points());
    return os;
}

std::ostream& operator<<(std::ostream& os, const std::shared_ptr<Soma>& somaPtr) {
    os << *somaPtr;
    return os;
}

}  // end namespace mut
}  // end namespace morphio
