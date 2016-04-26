
/* Copyright (c) 2006-2015, Raphael Dumusc <raphael.dumusc@epfl.ch>
 *                          Juan Hernando <jhernando@fi.upm.es>
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

#include "spikes.h"
#include "detail/spikes.h"

namespace brain
{

Spikes::Spikes()
    : _impl( new detail::Spikes( ))
{}

Spikes::Spikes( detail::Spikes* impl )
    : _impl( impl )
{}

Spikes::Spikes(const Spikes& rhs)
    : _impl( new detail::Spikes( *rhs._impl ))
{}

Spikes::~Spikes()
{
    delete _impl;
}

Spikes& Spikes::operator=(const Spikes& rhs)
{
    if (this == &rhs)
        return *this;

    delete _impl;
    _impl = new detail::Spikes( *rhs._impl );

    return *this;
}

Spikes::const_iterator Spikes::begin() const
{
    return const_iterator( new detail::Spikes_const_iterator( _impl->_begin ));
}

Spikes::const_iterator Spikes::end() const
{
    return const_iterator( new detail::Spikes_const_iterator( _impl->_end ));
}

float Spikes::getStartTime() const
{
    return _impl->_startTime;
}

float Spikes::getEndTime() const
{
    return _impl->_endTime;
}

size_t Spikes::size() const
{
    return _impl->_size;
}

bool Spikes::empty() const
{
    return _impl->_size == 0;
}

Spikes::const_iterator::const_iterator()
    : _impl( new detail::Spikes_const_iterator )
{}

Spikes::const_iterator::const_iterator(const Spikes::const_iterator& rhs)
    : _impl( new detail::Spikes_const_iterator( *rhs._impl ))
{}

Spikes::const_iterator& Spikes::const_iterator::operator= (
    const Spikes::const_iterator& rhs)
{
    if (this == &rhs)
        return *this;

    delete _impl;
    _impl = new detail::Spikes_const_iterator( *rhs._impl );

    return *this;
}

Spikes::const_iterator::~const_iterator()
{
    delete _impl;
}

Spikes::const_iterator::const_iterator( detail::Spikes_const_iterator* impl )
    : _impl( impl )
{}

brion::Spike Spikes::const_iterator::dereference() const
{
    return *(_impl->_it);
}

bool Spikes::const_iterator::equal( const Spikes::const_iterator& other ) const
{
    return _impl->_it == other._impl->_it;
}

void Spikes::const_iterator::increment()
{
    ++(_impl->_it);
}

void Spikes::const_iterator::decrement()
{
    --(_impl->_it);
}

}
