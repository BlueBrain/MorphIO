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
 * This container can be iterated as well as random accessed using the
 * operator [].
 *
 * This class is thread-safe, moveable and copyable.
 */
class Synapses
{
public:
    /** Frees all memory on destruction of last copy. */
    BRAIN_API ~Synapses();

    /** Conversion constructor for direct access from getXXXSynapses(). */
    BRAIN_API Synapses(const SynapsesStream& stream);

    /** @name Copy semantics by data sharing. */
    //@{
    BRAIN_API Synapses(const Synapses&);
    BRAIN_API Synapses& operator=(const Synapses&);
    //@}

    /** @name Move semantics. */
    //@{
    BRAIN_API Synapses(Synapses&& rhs);
    BRAIN_API Synapses& operator=(Synapses&& rhs);
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
    BRAIN_API Synapse operator[](size_t index) const;

    /**
     * @return the synapse GIDs containing GIDs of the post-synaptic cells and
     *         the indices in the afferent contacts array.
     * @throw std::runtime_error if index information not found in the synapse
     *                           source of the circuit.
     */
    BRAIN_API const size_t* indices() const;

    /** @name Presynaptic topological and geometrical attributes */
    //@{
    /** @return the GIDs of the presynaptic neurons. */
    BRAIN_API const uint32_t* preGIDs() const;

    /** @return the section IDs on the presynaptic neurons. */
    BRAIN_API const uint32_t* preSectionIDs() const;

    /** @return the segment IDs on the presynaptic neurons. */
    BRAIN_API const uint32_t* preSegmentIDs() const;

    /** @return the distances in micrometer to the presynaptic neurons. */
    BRAIN_API const float* preDistances() const;

    /**
     * @return the presynaptic touch position x-coordinates on the surfaces of
     *         the segments. May be null in old circuits.
     */
    BRAIN_API const float* preSurfaceXPositions() const;

    /**
     * @return the presynaptic touch position y-coordinates on the surfaces of
     *         the segments. May be null in old circuits.
     */
    BRAIN_API const float* preSurfaceYPositions() const;

    /**
     * @return the presynaptic touch position z-coordinates on the surfaces of
     *         the segments. May be null in old circuits.
     */
    BRAIN_API const float* preSurfaceZPositions() const;

    /**
     * @return the presynaptic touch position x-coordinates in the center of the
     * segments.
     */
    BRAIN_API const float* preCenterXPositions() const;

    /**
     * @return the presynaptic touch position y-coordinates in the center of the
     * segments.
     */
    BRAIN_API const float* preCenterYPositions() const;

    /**
     * @return the presynaptic touch position z-coordinates in the center of the
     * segments.
     */
    BRAIN_API const float* preCenterZPositions() const;
    //@}

    /** @name Presynaptic topological and geometrical attributes */
    //@{
    /** @return the GIDs of the postsynaptic neurons. */
    BRAIN_API const uint32_t* postGIDs() const;

    /** @return the section IDs on the postsynaptic neurons. */
    BRAIN_API const uint32_t* postSectionIDs() const;

    /** @return the segment IDs on the postsynaptic neurons. */
    BRAIN_API const uint32_t* postSegmentIDs() const;

    /** @return the distances in micrometer to the postsynaptic neurons. */
    BRAIN_API const float* postDistances() const;

    /**
     * @return the postsynaptic touch position x-coordinates on the surfaces of
     *         the segments. May be null in old circuits.
     */
    BRAIN_API const float* postSurfaceXPositions() const;

    /**
     * @return the postsynaptic touch position x-coordinates on the surfaces of
     *         the segments. May be null in old circuits.
     */
    BRAIN_API const float* postSurfaceYPositions() const;

    /**
     * @return the postsynaptic touch position x-coordinates on the surfaces of
     *         the segments. May be null in old circuits.
     */
    BRAIN_API const float* postSurfaceZPositions() const;

    /**
     * @return the postsynaptic touch position x-coordinates in the center of
     *         the segments.
     */
    BRAIN_API const float* postCenterXPositions() const;

    /**
     * @return the postsynaptic touch position y-coordinates in the center of
     *         the segments.
     */
    BRAIN_API const float* postCenterYPositions() const;

    /**
     * @return the postsynaptic touch position z-coordinates in the center of
     *         the segments.
     */
    BRAIN_API const float* postCenterZPositions() const;
    //@}

    /** @name Synaptic model attributes */
    //@{
    /** @return the axonal delays in milliseconds. */
    BRAIN_API const float* delays() const;

    /** @return the conductances in nanosiemens. */
    BRAIN_API const float* conductances() const;

    /** @return the neuro-transmitter release probabilities. */
    BRAIN_API const float* utilizations() const;

    /** @return the depression time constants in milliseconds. */
    BRAIN_API const float* depressions() const;

    /** @return the facilitation time constants in milliseconds. */
    BRAIN_API const float* facilitations() const;

    /** @return the decay time constants in milliseconds. */
    BRAIN_API const float* decays() const;

    /** @return the absolute synaptic efficacies in millivolts. */
    BRAIN_API const int* efficacies() const;
    //@}

protected:
    // The Impl pointer needs to be used in the Python wrapping as a custodian.
    // Instead of moving the Impl declaration outside the implementation, this
    // base class is used to give access to a virtual destructor, which is all
    // the wrapping needs.
    struct BaseImpl
    {
        virtual ~BaseImpl(){};
    };
    struct Impl;
    std::shared_ptr<const BaseImpl> _impl;

private:
    friend struct detail::SynapsesStream;
    Synapses(const Circuit& circuit, const GIDSet& gids,
             const GIDSet& filterGIDs, bool afferent, SynapsePrefetch prefetch);
};
}

#endif
