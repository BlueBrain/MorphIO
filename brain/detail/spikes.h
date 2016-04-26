
/* Copyright (c) 2006-2015, Juan Hernando <jhernando@fi.upm.es>
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
#ifndef BRAIN_DETAIL_SPIKES_H
#define BRAIN_DETAIL_SPIKES_H

namespace brain
{
namespace detail
{

class Spikes
{
public:
    /** Create a default Spikes private implementation. */
    Spikes()
        : _startTime(0)
        , _endTime(0)
        , _size(0)
    {}

    /** Create a Spikes private implementation. */
    Spikes( const brion::Spikes::const_iterator& begin,
            const brion::Spikes::const_iterator& end,
            const float startTime,
            const float endTime,
            const size_t size )
        : _begin( begin )
        , _end( end )
        , _startTime( startTime )
        , _endTime( endTime )
        , _size( size )
    {}

    brion::Spikes::const_iterator _begin;
    brion::Spikes::const_iterator _end;
    float _startTime;
    float _endTime;
    size_t _size;
};

class Spikes_const_iterator
{
public:
    /** Create a default / invalid iterator */
    Spikes_const_iterator() {}

    /** Constructor */
    explicit Spikes_const_iterator( const brion::Spikes::const_iterator& it )
        : _it( it )
    {}

    brion::Spikes::const_iterator _it;
};

}
}
#endif
