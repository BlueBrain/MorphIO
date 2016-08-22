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

#ifndef BRAIN_SYNAPSES
#define BRAIN_SYNAPSES

#include <brain/api.h>
#include <brain/types.h>

#include <memory>

namespace brain
{

/**
 * A container providing read-only access to Synapses retrieved by
 * getXXXSynapses() functions from brain::Circuit. It provides per-object and
 * per-array access on the various synapses attributes. Data which was not
 * prefetched will be loaded on-demand.
 *
 * This class is thread-safe, moveable and copyable.
 */
class Synapses
{
public:
    /** Frees all memory on destruction of last copy. */
    BRAIN_API ~Synapses();

    /** Conversion constructor for direct access from getXXXSynapses(). */
    BRAIN_API Synapses( const SynapsesStream& stream );

    /** @name Copy semantics by data sharing. */
    //@{
    BRAIN_API Synapses( const Synapses& );
    BRAIN_API Synapses& operator=( const Synapses& );
    //@}

    /** @name Move semantics. */
    //@{
    BRAIN_API Synapses( Synapses&& rhs );
    BRAIN_API Synapses& operator=( Synapses&& rhs );
    //@}

    /** @return number of synapses available in this container. */
    BRAIN_API size_t size() const;

    /** @return size() == 0. */
    BRAIN_API bool empty() const;

    using const_iterator = SynapsesIterator;

    /** @return an iterator to the first synapse of this container. */
    BRAIN_API const_iterator begin() const;

    /** @return an iterator to the past-the-end synapse of this container. */
    BRAIN_API const_iterator end() const;

    /** return the Synapse at the index-th position. */
    BRAIN_API Synapse operator[]( size_t index ) const;

    /**
     * @return the synapse GIDs containing GIDs of the post-synaptic cells and
     *         the indices in the afferent contacts array.
     * @throw std::runtime_error if index information not found in the synapse
     *                           source of the circuit.
     */
    BRAIN_API const size_t* index() const;

    /** @name Presynaptic topological and geometrical attributes */
    //@{
    /** @return the GIDs of the presynaptic neurons. */
    BRAIN_API const uint32_t* preGID() const;

    /** @return the section IDs on the presynaptic neurons. */
    BRAIN_API const uint32_t* preSectionID() const;

    /** @return the segment IDs on the presynaptic neurons. */
    BRAIN_API const uint32_t* preSegmentID() const;

    /** @return the distances in micrometer to the presynaptic neurons. */
    BRAIN_API const float* preDistance() const;

    /**
     * @return the presynaptic touch position x-coordinates on the surfaces of
     *         the segments.
     */
    BRAIN_API const float* preSurfacePositionX() const;

    /**
     * @return the presynaptic touch position y-coordinates on the surfaces of
     *         the segments.
     */
    BRAIN_API const float* preSurfacePositionY() const;

    /**
     * @return the presynaptic touch position z-coordinates on the surfaces of
     *         the segments.
     */
    BRAIN_API const float* preSurfacePositionZ() const;

    /**
     * @return the presynaptic touch position x-coordinates in the center of the
     * segments.
     */
    BRAIN_API const float* preCenterPositionX() const;

    /**
     * @return the presynaptic touch position y-coordinates in the center of the
     * segments.
     */
    BRAIN_API const float* preCenterPositionY() const;

    /**
     * @return the presynaptic touch position z-coordinates in the center of the
     * segments.
     */
    BRAIN_API const float* preCenterPositionZ() const;
    //@}

    /** @name Presynaptic topological and geometrical attributes */
    //@{
    /** @return the GIDs of the postsynaptic neurons. */
    BRAIN_API const uint32_t* postGID() const;

    /** @return the section IDs on the postsynaptic neurons. */
    BRAIN_API const uint32_t* postSectionID() const;

    /** @return the segment IDs on the postsynaptic neurons. */
    BRAIN_API const uint32_t* postSegmentID() const;

    /** @return the distances in micrometer to the postsynaptic neurons. */
    BRAIN_API const float* postDistance() const;

    /**
     * @return the postsynaptic touch position x-coordinates on the surfaces of
     *         the segments.
     */
    BRAIN_API const float* postSurfacePositionX() const;

    /**
     * @return the postsynaptic touch position x-coordinates on the surfaces of
     *         the segments.
     */
    BRAIN_API const float* postSurfacePositionY() const;

    /**
     * @return the postsynaptic touch position x-coordinates on the surfaces of
     *         the segments.
     */
    BRAIN_API const float* postSurfacePositionZ() const;

    /**
     * @return the postsynaptic touch position x-coordinates in the center of
     *         the segments.
     */
    BRAIN_API const float* postCenterPositionX() const;

    /**
     * @return the postsynaptic touch position y-coordinates in the center of
     *         the segments.
     */
    BRAIN_API const float* postCenterPositionY() const;

    /**
     * @return the postsynaptic touch position z-coordinates in the center of
     *         the segments.
     */
    BRAIN_API const float* postCenterPositionZ() const;
    //@}

    /** @name Synaptic model attributes */
    //@{
    /** @return the axonal delays in milliseconds. */
    BRAIN_API const float* delay() const;

    /** @return the conductances in nanosiemens. */
    BRAIN_API const float* conductance() const;

    /** @return the neuro-transmitter release probabilities. */
    BRAIN_API const float* utilization() const;

    /** @return the depression time constants in milliseconds. */
    BRAIN_API const float* depression() const;

    /** @return the facilitation time constants in milliseconds. */
    BRAIN_API const float* facilitation() const;

    /** @return the decay time constants in milliseconds. */
    BRAIN_API const float* decay() const;

    /** @return the absolute synaptic efficacies in millivolts. */
    BRAIN_API const int* efficacy() const;
    //@}

private:
    friend struct detail::SynapsesStream;
    Synapses( const Circuit& circuit, const GIDSet& gids,
              const GIDSet& filterGIDs, bool afferent, SynapsePrefetch prefetch );

    struct Impl;
    std::shared_ptr< const Impl > _impl;
};

}

#endif
