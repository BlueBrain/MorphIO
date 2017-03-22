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

#ifndef BRAIN_DETAIL_SYNAPSESSTREAM
#define BRAIN_DETAIL_SYNAPSESSTREAM

#include <brain/circuit.h>

#include <future>

namespace brain
{
namespace detail
{
struct SynapsesStream
{
    SynapsesStream(const Circuit& circuit, const GIDSet& gids,
                   const bool afferent, const SynapsePrefetch prefetch)
        : _circuit(circuit)
        , _afferent(afferent)
        , _gids(gids)
        , _prefetch(prefetch)
        , _it(_gids.begin())
    {
    }

    SynapsesStream(const Circuit& circuit, const GIDSet& preGIDs,
                   const GIDSet& postGIDs, const SynapsePrefetch prefetch)
        : _circuit(circuit)
        , _afferent(preGIDs.empty() || (postGIDs.size() < preGIDs.size()))
        , _gids(_afferent ? postGIDs : preGIDs)
        , _filterGIDs(_afferent ? preGIDs : postGIDs)
        , _prefetch(prefetch)
        , _it(_gids.begin())
    {
    }

    const Circuit& _circuit;
    const bool _afferent;
    const GIDSet _gids;
    const GIDSet _filterGIDs;
    const SynapsePrefetch _prefetch;
    GIDSet::const_iterator _it;

    size_t getRemaining() const
    {
        return size_t(std::abs(std::distance(_it, _gids.end())));
    }

    std::future<Synapses> read(size_t count)
    {
        count = std::min(count, getRemaining());
        GIDSet::const_iterator start = _it;
        std::advance(_it, count);
        GIDSet::const_iterator end = _it;
        return std::async(std::launch::async, [&, start, end] {
            return Synapses(_circuit, GIDSet(start, end), _filterGIDs,
                            _afferent, _prefetch);
        });
    }
};
}
}

#endif
