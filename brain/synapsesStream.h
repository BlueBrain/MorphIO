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

#ifndef BRAIN_SYNAPSESSTREAM
#define BRAIN_SYNAPSESSTREAM

#include <brain/api.h>
#include <brain/types.h>

#include <future>
#include <memory>

namespace brain
{
/**
 * A class which allows sequential and forward-only iterations through the
 * synapses from the involved GIDs retrieved by getXXXSynapses() functions from
 * brain::Circuit.
 *
 * This class is moveable, but non-copyable and not thread-safe.
 */
class SynapsesStream
{
public:
    BRAIN_API ~SynapsesStream();

    /** @name Move semantics. */
    //@{
    BRAIN_API SynapsesStream(SynapsesStream&& rhs);
    BRAIN_API SynapsesStream& operator=(SynapsesStream&& rhs);
    //@}

    /**
     * @return if the end of the stream was reached, i.e. any subsequent read()
     *         will return empty Synapses.
     */
    BRAIN_API bool eos() const;

    /** @return the remaining count values for read(). */
    BRAIN_API size_t getRemaining() const;

    /**
     * Starts an asynchronous read of the data for the next fraction of synapses
     * of the requested GIDs.
     *
     * @param count the next fraction in the [0,getRemaining()] interval to read
     * @return a future to the Synapses containing the requested fraction of
     *         synapses
     */
    BRAIN_API std::future<Synapses> read(size_t count = 1);

private:
    SynapsesStream(const SynapsesStream&) = delete;
    SynapsesStream& operator=(const SynapsesStream&) = delete;

    friend class Circuit;
    SynapsesStream(const Circuit& circuit, const GIDSet& gids, bool afferent,
                   SynapsePrefetch prefetch);
    SynapsesStream(const Circuit& circuit, const GIDSet& preGIDs,
                   const GIDSet& postGIDs, SynapsePrefetch prefetch);
    // Constructor for afferent external projections
    SynapsesStream(const Circuit& circuit, const GIDSet& gids,
                   const std::string& source, SynapsePrefetch prefetch);

    friend class Synapses;
    std::unique_ptr<detail::SynapsesStream> _impl;
};
}

#endif
