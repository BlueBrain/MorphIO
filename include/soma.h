
/* Copyright (c) 2013-2017, EPFL/Blue Brain Project
 *                          Juan Hernando <jhernando@fi.upm.es>
 *
 * This file is part of Brion <https://github.com/BlueBrain/Brion>
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 3.0 as published
 * by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef BRAIN_NEURON_SOMA
#define BRAIN_NEURON_SOMA

#include <types.h>

#include <morphology.h>

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


    BRAIN_API Point getSomaCenter();

    SectionType getType();

    Points getPoints(){ return get<Property::Point>(); }

    floats getDiameter(){ return get<Property::Diameter>(); }

private:
    template <typename Property> const typename Property::Type get() const;

    PropertiesPtr _properties;
    SectionRange _range;

};
}
#endif
