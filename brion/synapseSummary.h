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

#ifndef BBP_SYNAPSESUMMARY
#define BBP_SYNAPSESUMMARY

#include <boost/noncopyable.hpp>
#include <brion/api.h>
#include <brion/types.h>

namespace brion
{
namespace detail
{
class SynapseSummary;
}

/** Read access to a Synapse summary file.
 *
 * Following RAII, this class is ready to use after the creation and will ensure
 * release of resources upon destruction.
 */
class SynapseSummary : public boost::noncopyable
{
public:
    /** Close synapse summary file. @version 1.0 */
    BRION_API ~SynapseSummary();

    /** @name Read API */
    //@{
    /** Open the given source to a synapse summary file for reading.
     *
     * @param source filepath to synapse summary file
     * @throw std::runtime_error if file is not a valid synapse information file
     * @version 1.0
     */
    BRION_API explicit SynapseSummary(const std::string& source);

    /**
     * Read afferent and efferent information for a given neuron.
     *
     * @param gid GID of neuron, empty return value for not found GID
     * @return data matrix containing GID, number of efferent & afferent
     *         synapses for each connected neuron
     * @version 1.0
     */
    BRION_API SynapseSummaryMatrix read(const uint32_t gid) const;
    //@}

private:
    detail::SynapseSummary* const _impl;
};
}

#endif
