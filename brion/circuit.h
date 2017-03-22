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
#ifndef BRION_CIRCUIT
#define BRION_CIRCUIT

#include <boost/noncopyable.hpp>
#include <brion/api.h>
#include <brion/types.h>

namespace brion
{
/** Read access to a Circuit file.
 *
 * This class loads the circuit data at creation and will ensure release of
 * resources upon destruction.
 */
class Circuit : public boost::noncopyable
{
public:
    /** Close circuit file. @version 1.0 */
    BRION_API ~Circuit();

    /** @name Read API */
    //@{
    /** Open given filepath to a circuit file for reading.
     *
     * @param source filepath to circuit file
     * @throw std::runtime_error if file is not a valid circuit file
     * @version 1.0
     */
    BRION_API explicit Circuit(const std::string& source);

    /** Open given filepath to a circuit file for reading.
     *
     * @param source filepath to circuit file
     * @throw std::runtime_error if file is not a valid circuit file
     * @version 1.7
     */
    BRION_API explicit Circuit(const URI& source);

    /** Retrieve neuron attributes for set of neurons.
     *
     * @param gids set of neurons of interest; if empty, all neurons in the
     *             circuit file are considered
     * @param attributes bitset of brion::NeuronAttributes to load into return
     *                   value
     * @return data matrix containing values for the given attributes stored in
     *                     a consecutive way
     * @throw std::runtime_error if any GID is out of range.
     * @version 1.0
     */
    BRION_API NeuronMatrix get(const GIDSet& gids,
                               const uint32_t attributes) const;

    /** @return number of neurons stored in the circuit file. @version 1.0 */
    BRION_API size_t getNumNeurons() const;

    /** Get type information for the neurons in the circuit.
     *
     * @param type desired type of information
     * @return list of type information
     * @version 1.0
     */
    BRION_API Strings getTypes(const NeuronClass type) const;
    //@}

private:
    class Impl;
    Impl* const _impl;
};
}

#endif
