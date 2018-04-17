#include <cmath>

#include <morphio/mut/soma.h>
#include <morphio/soma.h>

#include <morphio/section.h>

namespace morphio
{
namespace mut
{

Soma::Soma(const Property::PointLevel &pointProperties) : _somaType(SOMA_UNDEFINED),
                                                          _pointProperties(pointProperties)
{
}

Soma::Soma(const morphio::Soma& soma) : _somaType(soma.type())
{
    _pointProperties =
        Property::PointLevel(soma._properties->_pointLevel, soma._range);
}

const float Soma::surface() const {
    switch(type()) {
    case SOMA_NEUROMORPHO_THREE_POINT_CYLINDERS:
    {
        float radius = diameters()[0] / 2;
        return 4 * M_PI * radius * radius;
    }
    case SOMA_SIMPLE_CONTOUR:
    {
        std::vector<Point> polygone;
        polygone = points();

        polygone.push_back(polygone[0]);

        float area = 0;

        for(int i = 1;i<polygone.size(); ++i){
            float x_i = polygone[i][0];
            float y_i = polygone[i][1];
            float x_prev_i = polygone[i-1][0];
            float y_prev_i = polygone[i-1][1];
            area += 0.5 * (x_prev_i * y_i - x_i * y_prev_i);
        }
        return area;
    }

    default:
        throw std::runtime_error("Soma::surface is not implemented for this soma type");
    }
}

std::ostream& operator<<(std::ostream& os, Soma& soma)
{
    os << dumpPoints(soma.points());
    return os;
}

std::ostream& operator<<(std::ostream& os, std::shared_ptr<Soma> somaPtr){
    os << *somaPtr;
    return os;
}

} // end namespace mut
} // end namespace morphio
