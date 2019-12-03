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
    Soma(const Soma& soma);
    Soma(const morphio::Soma& soma);

    ~Soma() {}
    /**
       Return the coordinates (x,y,z) of all soma point
    **/
    std::vector<Point>& points() noexcept { return _pointProperties._points; }

    const std::vector<Point>& points() const noexcept
    {
        return _pointProperties._points;
    }

    /**
       Return the diameters of all soma points
    **/
    std::vector<float>& diameters() noexcept { return _pointProperties._diameters; }
    const std::vector<float>& diameters() const noexcept
    {
        return _pointProperties._diameters;
    }

    /**
       Return the soma type
    **/
    SomaType type() const noexcept { return _somaType; }
    /**
     * Return the center of gravity of the soma points
     **/
    Point center() const;

    /**
       Return the soma surface
       Note: the soma surface computation depends on the soma type
    **/
    float surface() const;

    /**
     * Return the maximum distance between the center of gravity and any of
     * the soma points
     */
    float maxDistance() const;

    Property::PointLevel& properties() { return _pointProperties; }
private:
    friend class Morphology;
    SomaType _somaType;
    Property::PointLevel _pointProperties;
};

std::ostream& operator<<(std::ostream& os, const std::shared_ptr<Soma>& sectionPtr);
std::ostream& operator<<(std::ostream& os, const Soma& soma);

} // namespace mut
} // namespace morphio
