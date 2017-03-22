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

#include "synapsesIterator.h"

#include "synapse.h"
#include "synapses.h"

namespace brain
{
SynapsesIterator::SynapsesIterator(const Synapses& synapses, const size_t index)
    : _synapses(synapses)
    , _index(index)
{
}

SynapsesIterator::~SynapsesIterator()
{
}

bool SynapsesIterator::operator==(const SynapsesIterator& rhs) const
{
    return _index == rhs._index;
}

bool SynapsesIterator::operator!=(const SynapsesIterator& rhs) const
{
    return _index != rhs._index;
}

SynapsesIterator& SynapsesIterator::operator++()
{
    ++_index;
    return *this;
}

Synapse SynapsesIterator::operator*() const
{
    return _synapses[_index];
}
}
