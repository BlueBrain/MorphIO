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

Soma::Soma(const Soma& soma)
    : _somaType(soma._somaType)
    , _pointProperties(soma._pointProperties)
{
}

Soma::Soma(const morphio::Soma& soma)
    : _somaType(soma.type())
    , _pointProperties(soma._properties->_somaLevel)
{
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

float Soma::maxDistance() const
{
    const auto ps = points();
    if (ps.size() == 0) {
        return 0.;
    }
    const auto c = center();
    float dist = distance(c, ps[0]);
    for (size_t i = 1; i < ps.size(); ++i) {
        const float d = distance(c, ps[i]);
        if (d > dist) {
            dist = d;
        }
    }
    return dist;
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
