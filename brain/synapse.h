/* Copyright (c) 2013-2016, EPFL/Blue Brain Project
 *                          Daniel.Nachbaur@epfl.ch
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

#ifndef BRAIN_SYNAPSE
#define BRAIN_SYNAPSE

#include <brain/api.h>
#include <brain/types.h>

namespace brain
{
/**
 * A proxy object returned by the Synapses container to access data for a
 * particular synapse.
 *
 * The lifetime of this object is stricly bound to the synapses container it
 * comes from.
 */
class Synapse
{
public:
    BRAIN_API ~Synapse();

    /**
     * @return the synapse GID containing GID of the post-synaptic cell and the
     *         index in the afferent contacts array.
     * @throw std::runtime_error if index information not found in the synapse
     *                           source of the circuit.
     */
    BRAIN_API SynapseGID getGID() const;

    /** @name Presynaptic topological and geometrical attributes */
    //@{
    /** @return the GID of the presynaptic neuron. */
    BRAIN_API uint32_t getPresynapticGID() const;

    /** @return the section ID on the presynaptic neuron. */
    BRAIN_API uint32_t getPresynapticSectionID() const;

    /** @return the segment ID on the presynaptic neuron. */
    BRAIN_API uint32_t getPresynapticSegmentID() const;

    /** @return the distance from the beginning of the presynaptic segment
                to the synapse in micrometers. */
    BRAIN_API float getPresynapticDistance() const;

    /** @return the presynaptic touch position on the surface of the segment. */
    BRAIN_API Vector3f getPresynapticSurfacePosition() const;

    /** @return the presynaptic touch position in the center of the segment. */
    BRAIN_API Vector3f getPresynapticCenterPosition() const;
    //@}

    /** @name Presynaptic topological and geometrical attributes */
    //@{
    /** @return the GID of the postsynaptic neuron. */
    BRAIN_API uint32_t getPostsynapticGID() const;

    /** @return the section ID on the postsynaptic neuron. */
    BRAIN_API uint32_t getPostsynapticSectionID() const;

    /** @return the segment ID on the postsynaptic neuron. */
    BRAIN_API uint32_t getPostsynapticSegmentID() const;

    /** @return the distance from the beginning of the postsynaptic segment
                to the synapse in micrometers. */
    BRAIN_API float getPostsynapticDistance() const;

    /** @return the postsynaptic touch position on the surface of the segment.*/
    BRAIN_API Vector3f getPostsynapticSurfacePosition() const;

    /** @return the postsynaptic touch position in the center of the segment. */
    BRAIN_API Vector3f getPostsynapticCenterPosition() const;
    //@}

    /** @name Synaptic model attributes */
    //@{
    /** @return the axonal delay in milliseconds. */
    BRAIN_API float getDelay() const;

    /** @return the conductance in nanosiemens. */
    BRAIN_API float getConductance() const;

    /** @return the neuro-transmitter release probability. */
    BRAIN_API float getUtilization() const;

    /** @return the depression time constant in milliseconds. */
    BRAIN_API float getDepression() const;

    /** @return the facilitation time constant in milliseconds. */
    BRAIN_API float getFacilitation() const;

    /** @return the decay time constant in milliseconds. */
    BRAIN_API float getDecay() const;

    /** @return the absolute synaptic efficacy in millivolts. */
    BRAIN_API int getEfficacy() const;
    //@}

private:
    friend class Synapses;
    Synapse(const Synapses& synapses, size_t index);

    const Synapses& _synapses;
    const size_t _index;
};
}

#endif
