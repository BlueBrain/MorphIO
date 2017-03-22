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
 * Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "synapsesStream.h"
#include "synapses.h"

#include "detail/synapsesStream.h"

namespace brain
{
SynapsesStream::SynapsesStream(const Circuit& circuit, const GIDSet& gids,
                               const bool afferent,
                               const SynapsePrefetch prefetch)
    : _impl(new detail::SynapsesStream(circuit, gids, afferent, prefetch))
{
}

SynapsesStream::SynapsesStream(const Circuit& circuit, const GIDSet& preGIDs,
                               const GIDSet& postGIDs,
                               const SynapsePrefetch prefetch)
    : _impl(new detail::SynapsesStream(circuit, preGIDs, postGIDs, prefetch))
{
}

SynapsesStream::~SynapsesStream()
{
}

SynapsesStream::SynapsesStream(SynapsesStream&& rhs)
    : _impl(std::move(rhs._impl))
{
}

SynapsesStream& SynapsesStream::operator=(SynapsesStream&& rhs)
{
    if (this != &rhs)
        _impl = std::move(rhs._impl);
    return *this;
}

bool SynapsesStream::eos() const
{
    return _impl->_it == _impl->_gids.end();
}

size_t SynapsesStream::getRemaining() const
{
    return _impl->getRemaining();
}

std::future<Synapses> SynapsesStream::read(const size_t count)
{
    return _impl->read(count);
}
}
