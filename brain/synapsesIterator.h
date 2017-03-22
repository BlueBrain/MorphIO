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

#ifndef BRAIN_SYNAPSES_ITERATOR
#define BRAIN_SYNAPSES_ITERATOR

#include <brain/api.h>
#include <brain/types.h>

namespace brain
{
/** A forward iterator for the Synapses container. */
class SynapsesIterator
    : public std::iterator<std::forward_iterator_tag, Synapse>
{
public:
    BRAIN_API SynapsesIterator(const Synapses& synapses, size_t index);
    BRAIN_API ~SynapsesIterator();

    BRAIN_API bool operator==(const SynapsesIterator& rhs) const;
    BRAIN_API bool operator!=(const SynapsesIterator& rhs) const;

    BRAIN_API SynapsesIterator& operator++();

    BRAIN_API Synapse operator*() const;

private:
    const Synapses& _synapses;
    size_t _index;
};
}

#endif
