#pragma once

#include <morphio/properties.h>

namespace morphio
{
namespace mut
{
class Soma
{
public:
    Soma()
        {
        }
    Soma(Property::PointLevel pointProperties):
        _pointProperties(pointProperties)
        {
        }

    Soma(const morphio::Soma& soma);
    std::vector<Point>& points() { return _pointProperties._points; }
    std::vector<float> diameters() { return _pointProperties._diameters; }
private:
    friend class Morphology;
    Property::PointLevel _pointProperties;
};
} // namespace mut
} // namespace morphio
