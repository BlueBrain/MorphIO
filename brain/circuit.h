/* Copyright (c) 2013-2016, EPFL/Blue Brain Project
 *                          Juan Hernando <jhernando@fi.upm.es>
 *                          Adrien.Devresse@epfl.ch
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
#include <brain/neuron/types.h>
#include <brain/types.h>

#include <memory>
#include <vmmlib/matrix.hpp> // return value

namespace brain
{
/** Read access to a circuit database
 *
 * This class provides convenience functions to access information about the
 * cells inside the circuit and their morphologies.
 */
class Circuit
{
public:
    /** Coordinate system to use for circuit morphologies */
    enum class Coordinates
    {
        global,
        local
    };

    /**
     * Opens a circuit for read access.
     *
     * @param source the URI to the CircuitConfig or BlueConfig file.
     */
    BRAIN_API explicit Circuit(const URI& source);

    /**
     * Opens a circuit for read access.
     * @param blueConfig The object representing the BlueConfig.
     */
    BRAIN_API explicit Circuit(const brion::BlueConfig& blueConfig);

    BRAIN_API ~Circuit();

    /**
     * @return The \if pybind array \else set \endif of GIDs for the given
     *         target name.
     * @throw std::runtime_error if the target cannot be found.
     */
    BRAIN_API GIDSet getGIDs(const std::string& target) const;

    /** @return The \if pybind array \else set \endif of all GIDs held by the
     *          circuit.
     */
    BRAIN_API GIDSet getGIDs() const;

    /**
     * @return A random fraction of GIDs from the given target name.
     * @env BRAIN_CIRCUIT_SEED set the seed for deterministic randomness
     * @throw std::runtime_error if the fraction is not in the range [0,1].
     * @throw std::runtime_error if the target cannot be found.
     */
    BRAIN_API GIDSet getRandomGIDs(float fraction,
                                   const std::string& target) const;

    /**
     * @return A random fraction of GIDs from the circuit.
     * @env BRAIN_CIRCUIT_SEED set the seed for deterministic randomness
     * @throw std::runtime_error if the fraction is not in the range [0,1].
     */
    BRAIN_API GIDSet getRandomGIDs(float fraction) const;

    /** @return The set of URIs to access the morphologies of the given cells */
    BRAIN_API URIs getMorphologyURIs(const GIDSet& gids) const;

    /**
     * @return The list of morpholgies for the GID set. If local coordinates
     *         are requested, morphologies that are repeated in the circuit
     *         will shared the same Morphology object in the list. If global
     *         coordinates are requested, all Morphology objects are unique.
     */
    BRAIN_API neuron::Morphologies loadMorphologies(const GIDSet& gids,
                                                    Coordinates coords) const;

    /** @return The positions of the given cells in the iteration order of the
     *          input gids.
     */
    BRAIN_API Vector3fs getPositions(const GIDSet& gids) const;

    /** @return The morphology type indices of the given cells in the iteration
     *          order of the input gids.
     */
    BRAIN_API size_ts getMorphologyTypes(const GIDSet& gids) const;

    /**
     * @return The morphology type names of the circuit, indexed by
     *         getMorphologyTypes().
     */
    BRAIN_API Strings getMorphologyTypeNames() const;

    /** @return The electrophysiology type indices of the given cells in the
     *          iteration order of the input gids.
     */
    BRAIN_API size_ts getElectrophysiologyTypes(const GIDSet& gids) const;

    /**
     * @return The electrophysiology type names of the circuit, indexed by
     *         getElectrophysiologyTypes().
     */
    BRAIN_API Strings getElectrophysiologyTypeNames() const;

    /** @return \if pybind A Nx4x4 numpy array with the \else The \endif
     *          local to world transformations of the given cells in their
     *          iteration order.
     */
    BRAIN_API Matrix4fs getTransforms(const GIDSet& gids) const;

    /** @return \if pybind A Nx4 numpy array with the \else The \endif
     *          local to world rotation of the given cells.
     */
    BRAIN_API Quaternionfs getRotations(const GIDSet& gids) const;

    /** @return The number of neurons in the circuit. */
    BRAIN_API size_t getNumNeurons() const;

    /**
     * Access all afferent synapses of the given GIDs.
     *
     * @param gids the gids to load afferent synapses for
     * \if pybind
     * @param prefetch which synapse data to preload
     * \else
     * @param prefetch which synapse data to load on SynapsesStream.read()
     * @return synapse data stream
     * \endif
     */
    BRAIN_API SynapsesStream getAfferentSynapses(
        const GIDSet& gids,
        SynapsePrefetch prefetch = SynapsePrefetch::none) const;

    /**
     * Access all afferent synapses projected from another circuit into the
     * given GIDs.
     *
     * @param gids the gids of the post-synaptic cells
     * @param source the name of the projecting circuit. This corresponds
     *        to the label of a Projection section in the CircuitConfig.
     *        If the source doesn't exist an exception will be thrown by the
     *        first operation that tries to access the data.
     * \if pybind
     * @param prefetch which synapse data to preload
     * \else
     * @param prefetch which synapse data to load on SynapsesStream.read()
     * @return synapse data stream. The pre-synaptic GIDs in the synapses refer
     *         to cells in the projecting circuit.
     * \endif
     */
    BRAIN_API SynapsesStream getExternalAfferentSynapses(
        const GIDSet& gids, const std::string& source,
        SynapsePrefetch prefetch = SynapsePrefetch::none) const;

    /**
     * Access all efferent synapses of the given GIDs.
     *
     * @param gids the gids to load efferent synapses for
     * \if pybind
     * @param prefetch which synapse data to preload
     * \else
     * @param prefetch which synapse data to load on SynapsesStream.read()
     * @return synapse data stream
     * \endif
     */
    BRAIN_API SynapsesStream getEfferentSynapses(
        const GIDSet& gids,
        SynapsePrefetch prefetch = SynapsePrefetch::none) const;

    /**
     * Access all synapses along the projection from the pre- to the postGIDs.
     *
     * @param preGIDs the gids to load the efferent synapses for
     * @param postGIDs the gids to load the afferent synapses for
     * \if pybind
     * @param prefetch which synapse data to preload
     * \else
     * @param prefetch which synapse data to load on SynapsesStream.read()
     * @return synapse data stream
     * \endif
     */
    BRAIN_API SynapsesStream getProjectedSynapses(
        const GIDSet& preGIDs, const GIDSet& postGIDs,
        SynapsePrefetch prefetch = SynapsePrefetch::none) const;

    class Impl; //!< @internal, public for inheritance MVD2/3 impls

private:
    Circuit(const Circuit&) = delete;
    Circuit& operator=(const Circuit&) = delete;

    friend class Synapses;
    std::unique_ptr<const Impl> _impl;
};
}
#endif
