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

#ifndef BRAIN_CIRCUIT
#define BRAIN_CIRCUIT

#include <brain/api.h>
#include <brain/types.h>

#include <boost/noncopyable.hpp>

namespace brain
{

/** Read access to a circuit database
 *
 * This class provides convenience functions to access information about the
 * cells inside the circuit and their morphologies.
 */
class Circuit : public boost::noncopyable
{
public:
    /** Coordinate system to use for circuit morphologies */
    enum Coordinates
    {
        COORDINATES_GLOBAL,
        COORDINATES_LOCAL
    };

    /**
     * Opens a circuit for read access.
     * @param source the URI to the CircuitConfig or BlueConfig file.
     */
    BRAIN_API explicit Circuit( const URI& source );

    /**
     * Opens a circuit for read access.
     * @param blueConfig The object representing the BlueConfig.
     */
    BRAIN_API explicit Circuit( const brion::BlueConfig& blueConfig );

    BRAIN_API  ~Circuit();

    /**
     * @return The set of GIDs for the given target name. If empty it will
     *         return the circuit target specified on the
     *         BlueConfig/CircuitConfig file.
     *         If the target cannot be found or an empty string was given and
     *         there is no circuit target, the return value is an empty set.
     */
    BRAIN_API GIDSet getGIDs( const std::string& target = "" ) const;

    /** @return The set of URIs to access the morphologies of the given cells */
    BRAIN_API URIs getMorphologyURIs( const GIDSet& gids ) const;

    /**
     * @return The list of morpholgies for the GID set. If local coordinates
     *         are requested, morpholgies that are repeated in the circuit
     *         will shared the same Morphology object in the list. If global
     *         coordinates are requested, all Morphology objects are unique.
     */
    BRAIN_API Morphologies loadMorphologies( const GIDSet& gids,
                                             Coordinates coords ) const;

    /** @return The positions of the given cells. */
    BRAIN_API Vector3fs getPositions( const GIDSet& gids ) const;
    /** @return The local to world transformations of the given cells. */
    BRAIN_API Matrix4fs getTransforms( const GIDSet& gids ) const;

private:
    class Impl;
    Impl* _impl;
};

}
#endif
