#pragma once


#include <morphio/morphology.h>
#include <morphio/types.h>

namespace morphio
{
/**
 * A class to represent a neuron soma.
 *
 * This class provides functions to query information about the soma of a
 * neuron.
 *
 * Typically the soma is described as the poly-line of the projection
 * of the soma onto a plane, where the plane normal points in the vertical
 * direction in the local coordinate system of the morphology. In other cases
 * the poly-line is not projected onto a plane, but is an approximation of
 * the countour of the soma as seen in an orhogonal projection down the
 * vertical axis (this is basically the same as before, but the vertical
 * coordinate is not 0 for all the points).
 * This class can also be used for both descriptions as well as somas simply
 * approximated as spheres.
 *
 * The coordinates system used by a soma will be in the same as the
 * brain::Morphology from where it comes.
 *
 * @version unstable
 */
class Soma
{
public:

    /**
     * Return the center of gravity of the soma points
     **/
    const Point somaCenter() const;

    /**
     * Return the  coordinates (x,y,z) of all soma points
     **/
    const range<const Point> points() const
    {
        return get<Property::Point>();
    }

    /**
     * Return the diameters of all soma points
     **/
    const range<const float> diameters() const
    {
        return get<Property::Diameter>();
    }

    /**
     * Return the soma type
     **/
    const SomaType type() const
    {
        return _properties->_cellLevel._somaType;
    }

    /**
     * Return the soma volume\n"
     * Note: the soma volume computation depends on the soma type
     **/
    const float volume() const;

    /**
     * Return the soma surface\n"
     * Note: the soma surface computation depends on the soma type
     **/
    const float surface() const;
    const float maxDistance() const;



private:
    Soma(std::shared_ptr<Property::Properties>);
    template <typename Property>
    const range<const typename Property::Type> get() const;
    friend const Soma Morphology::soma() const;
    friend class mut::Soma;

    std::shared_ptr<Property::Properties> _properties;
    SectionRange _range;
};
}
