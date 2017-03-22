/* Copyright (c) 2013-2015, EPFL/Blue Brain Project
 *                          Daniel Nachbaur <daniel.nachbaur@epfl.ch>
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

#ifndef BRION_SYNAPSE
#define BRION_SYNAPSE

#include <boost/noncopyable.hpp>
#include <brion/api.h>
#include <brion/types.h>

namespace brion
{
namespace detail
{
class Synapse;
}

/** Read access to a Synapse file.
 *
 * There are three types of synapse files that can be read by this class:
 * - nrn[_efferent].h5: These files store the synaptic model parameters,
 *   connectivity information and morphological location of synapses. They
 *   are organized as sets of tables with one table per neuron and one row
 *   per afferent (or efferent) synapse.
 * - nrn_positions_[_efferent].h5: These files store precomputed spatial
 *   locations of synapses on the dendrite/axon longitudinal axes and on the
 *   cell membranes.
 * - nrn_extra.h5: This file stores the index of each synapse in the
 *   list of afferent synapses of a post-synaptic neuron before any pruning
 *   or filtering is carried out. This indices together with post-synaptic
 *   neuron GIDs can be used as synapse GIDs.
 *
 * There exists a forth file type, nrn_summary.h5, with the aggregated
 * connectivity per cell. SynapseSummary is the clase used to parse it.
 *
 * Following RAII, this class is ready to use after the creation and will ensure
 * release of resources upon destruction. Threadsafety is guaranteed for all
 * provided methods.
 *
 * Example: @include tests/synapse.cpp
 */
class Synapse : public boost::noncopyable
{
public:
    /** Close synapse file. @version 1.0 */
    BRION_API ~Synapse();

    /** @name Read API */
    //@{
    /** Open the given source to a synapse file for reading.
     *
     * @param source filepath to synapse file
     * @throw std::runtime_error if file is not a valid synapse file
     * @version 1.0
     */
    BRION_API explicit Synapse(const std::string& source);

    /** Read requested synapse attributes for a given neuron.
     *
     * @param gid GID of neuron, empty return value for not found GID
     * @param attributes bitwise combination of SynapseAttributes to load
     * @return data matrix containing values for attributes consecutively for
     *         each connected neuron
     * @version 1.0
     */
    BRION_API SynapseMatrix read(const uint32_t gid,
                                 const uint32_t attributes) const;

    /** Get the number of synapses for a set of neurons.
     *
     * @param gids GIDSet of neurons to consider
     * @return number of synapses for the set of neurons.
     * @version 1.0
     */
    BRION_API size_t getNumSynapses(const GIDSet& gids) const;

    /** Return the number of columns of the synapse file.
     * @version 1.9
     */
    BRION_API size_t getNumAttributes() const;
    //@}

private:
    detail::Synapse* const _impl;
};
}

#endif
