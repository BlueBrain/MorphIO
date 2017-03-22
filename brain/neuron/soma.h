
/* Copyright (c) 2013-2015, EPFL/Blue Brain Project
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

#include <brain/api.h>
#include <brain/types.h>

#include <brain/neuron/morphology.h>

namespace brain
{
namespace neuron
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
    friend class Morphology;

    ~Soma();

    BRAIN_API Soma(const Soma& soma);

    BRAIN_API Soma& operator=(const Soma& soma);

    /** Return the x,y,z and radius of the points of the soma
      * profile \if pybind as a 4xN numpy array\endif.
      */
    BRAIN_API Vector4fs getProfilePoints() const;

    /** Return the mean distance between the profile points and the centroid. */
    BRAIN_API float getMeanRadius() const;

    /** Return the average of the profile points. */
    BRAIN_API Vector3f getCentroid() const;

    /** Return the first order sections starting from the soma. */
    BRAIN_API Sections getChildren() const;

protected:
    BRAIN_API explicit Soma(Morphology::Impl* morhology);

private:
    Morphology::Impl* _morphology;
};
}
}
#endif
