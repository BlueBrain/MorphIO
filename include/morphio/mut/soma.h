#pragma once

#include <morphio/properties.h>

namespace morphio {
namespace mut {
class Soma
{
public:
    Soma()
        : _somaType(SOMA_UNDEFINED)
    {
    }
    Soma(const Property::PointLevel& pointProperties);
    Soma(const morphio::Soma& soma);

    ~Soma() {}

    /**
       Return the coordinates (x,y,z) of all soma point
    **/
    std::vector<Point>& points() { return _pointProperties._points; }
    const std::vector<Point>& points() const
    {
        return _pointProperties._points;
    }

    /**
       Return the diameters of all soma points
    **/
    std::vector<float>& diameters() { return _pointProperties._diameters; }
    const std::vector<float>& diameters() const
    {
        return _pointProperties._diameters;
    }

    /**
       Return the soma type
    **/
    SomaType type() const { return _somaType; }

    /**
     * Return the center of gravity of the soma points
     **/
    const Point center() const;

    /**
       Return the soma surface
       Note: the soma surface computation depends on the soma type
    **/
    float surface() const;

    Property::PointLevel& properties() { return _pointProperties; }

private:
    friend class Morphology;
    SomaType _somaType;
    Property::PointLevel _pointProperties;
};

std::ostream& operator<<(std::ostream& os, std::shared_ptr<Soma> sectionPtr);
std::ostream& operator<<(std::ostream& os, Soma& soma);

} // namespace mut
} // namespace morphio
