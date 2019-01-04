#include <cmath>

#include <morphio/section.h>
#include <morphio/shared_utils.tpp>
#include <morphio/soma.h>
#include <morphio/vector_types.h>

namespace morphio {
Soma::Soma(std::shared_ptr<Property::Properties> properties)
    : _properties(properties)
{
}

const Point Soma::center() const
{
    return centerOfGravity(_properties->_somaLevel._points);
}

float Soma::volume() const
{
    switch (_properties->_cellLevel._somaType) {
    case SOMA_NEUROMORPHO_THREE_POINT_CYLINDERS: {
        float radius = diameters()[0] / 2;
        return 4 * M_PI * radius * radius;
    }

    default:
        throw;
    }
}

float Soma::surface() const
{
    return _somaSurface<range<const float>, range<const Point>>(type(),
        diameters(),
        points());
}

float Soma::maxDistance() const
{
    return 0;
};

} // namespace morphio
