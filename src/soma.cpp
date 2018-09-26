#include <cmath>

#include <morphio/section.h>
#include <morphio/soma.h>
#include <morphio/vector_types.h>
#include <morphio/shared_utils.tpp>


namespace morphio
{
Soma::Soma(std::shared_ptr<Property::Properties> properties)
    : _properties(properties)
{
}

const Point Soma::center() const
{
    return centerOfGravity(_properties->_somaLevel._points);
}

const float Soma::volume() const {
    switch(_properties->_cellLevel._somaType) {
    case SOMA_NEUROMORPHO_THREE_POINT_CYLINDERS:
    {
        float radius = diameters()[0] / 2;
        return 4 * M_PI * radius * radius;
    }

    default:
        throw;

    }
}

const float Soma::surface() const {
    return _somaSurface<morphio::range<const float>,
                 morphio::range<const Point>>(type(), diameters(), points());
}

const float Soma::maxDistance() const {};


}
