#include <cmath>

#include <morphio/mut/soma.h>
#include <morphio/soma.h>

#include <morphio/section.h>
#include <morphio/shared_utils.tpp>

namespace morphio {
namespace mut {
Soma::Soma(const Property::PointLevel& pointProperties)
    : _somaType(SOMA_UNDEFINED)
    , _pointProperties(pointProperties)
{
}

Soma::Soma(const morphio::Soma& soma)
    : _somaType(soma.type())
{
    _pointProperties = Property::PointLevel(soma._properties->_somaLevel);
}

const Point Soma::center() const
{
    return centerOfGravity(points());
}

float Soma::surface() const
{
    return _somaSurface<std::vector<float>, std::vector<Point>>(type(),
        diameters(),
        points());
}

std::ostream& operator<<(std::ostream& os, Soma& soma)
{
    os << dumpPoints(soma.points());
    return os;
}

std::ostream& operator<<(std::ostream& os, std::shared_ptr<Soma> somaPtr)
{
    os << *somaPtr;
    return os;
}

} // end namespace mut
} // end namespace morphio
