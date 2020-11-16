#pragma once

#include <morphio/properties.h>

namespace morphio {
namespace mut {
class Soma
{
  public:
    Soma()
        : _somaType(SOMA_UNDEFINED) {}

    Soma(const Property::PointLevel& pointProperties);
    Soma(const Soma& soma);
    Soma(const morphio::Soma& soma);

    /** @{
       Return the coordinates (x,y,z) of all soma point
    **/
    inline std::vector<Point>& points() noexcept;
    inline const std::vector<Point>& points() const noexcept;
    /** @} */

    /**
       Return the diameters of all soma points
    **/
    inline std::vector<morphio::floatType>& diameters() noexcept;
    inline const std::vector<morphio::floatType>& diameters() const noexcept;
    /** @} */

    /**
       Return the soma type
    **/
    inline SomaType type() const noexcept;
    /**
     * Return the center of gravity of the soma points
     **/
    Point center() const;

    /**
       Return the soma surface
       Note: the soma surface computation depends on the soma type
    **/
    floatType surface() const;

    /**
     * Return the maximum distance between the center of gravity and any of
     * the soma points
     */
    floatType maxDistance() const;

    inline Property::PointLevel& properties() noexcept;
    inline const Property::PointLevel& properties() const noexcept;

  private:
    friend class Morphology;
    SomaType _somaType;
    Property::PointLevel _pointProperties;
};

inline std::vector<Point>& Soma::points() noexcept {
    return _pointProperties._points;
}

const std::vector<Point>& Soma::points() const noexcept {
    return _pointProperties._points;
}

inline std::vector<morphio::floatType>& Soma::diameters() noexcept {
    return _pointProperties._diameters;
}

const std::vector<morphio::floatType>& Soma::diameters() const noexcept {
    return _pointProperties._diameters;
}

inline SomaType Soma::type() const noexcept {
    return _somaType;
}

inline Property::PointLevel& Soma::properties() noexcept {
    return _pointProperties;
}

inline const Property::PointLevel& Soma::properties() const noexcept {
    return _pointProperties;
}

std::ostream& operator<<(std::ostream& os, const std::shared_ptr<Soma>& sectionPtr);
std::ostream& operator<<(std::ostream& os, const Soma& soma);

}  // namespace mut
}  // namespace morphio
