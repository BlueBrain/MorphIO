#pragma once

#include <gsl/span>

#include <minimorph/types.h>
#include <minimorph/morphology.h>

namespace minimorph
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
    BRAIN_API Soma(PropertiesPtr);

    BRAIN_API const Point getSomaCenter();
    BRAIN_API const SectionType getType();
    const Sections getRootSections() const;
    BRAIN_API const gsl::span<const Point> getPoints(){ return get<Property::Point>(); }
    BRAIN_API const gsl::span<const float> getDiameter(){ return get<Property::Diameter>(); }

private:
    template <typename Property> const gsl::span<const typename Property::Type> get() const;

    friend class builder::Soma;
    PropertiesPtr _properties;
    SectionRange _range;

};
}
