
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

#ifndef BRAIN_SPIKES_H
#define BRAIN_SPIKES_H

#include <brion/types.h>

#include <boost/iterator/iterator_facade.hpp>

namespace brain
{

namespace detail
{
    class Spikes;
    class Spikes_const_iterator;
}

/**
 * An iterable list of spikes over a time window, sorted by time.
 *
 * It is meant to be subclassed to provide different implementations for
 * different sources of data (file or stream).
 * By hiding the internal data structures, this object ensures that no
 * unnecessary copies of spikes data takes place regardless of the source type.
 */
class Spikes
{
public:
    /* Forward declaration. */
    class const_iterator;

    /**
     * Create an empty Spikes object.
     * @version 0.2
     */
    Spikes();

    /**
     * Copy constructor.
     * @version 0.2
     */
    Spikes( const Spikes& rhs );

    /**
     * Destructor
     * @version 0.2
     */
    ~Spikes();

    /**
     * Assignment operator.
     * @version 0.2
     */
    Spikes& operator= (const Spikes& rhs);

    /**
     * Returns the first element of the container for const linear access.
     * @version 0.2
     */
    const_iterator begin() const;

    /**
     * Returns the const iterator that indicates the end of the container.
     * @version 0.2
     */
    const_iterator end() const;

    /**
     * Get the start of the time window in milliseconds.
     *
     * This time may be smaller or equal than the smallest spike time
     * in the container.
     * @version 0.2
     */
    float getStartTime() const;

    /**
     * Get the end of the time window in milliseconds.
     *
     * This time may be greater or equal than the highest spike time
     * in the container
     * @version 0.2
     */
    float getEndTime() const;

    /**
     * Get the number of elements in this container.
     * @version 0.2
     */
    size_t size() const;

    /**
     * Check if the container is empty.
     * @version 0.2
     */
    bool empty() const;

protected:
    /**
     * Protected constructor.
     * Allows different implementations for different source types.
     * @param impl The private implementation (for ABI compatiblity).
     * @version 0.2
     */
    explicit Spikes( detail::Spikes* impl );

private:
    detail::Spikes* _impl;
};

/**
 * Iterator for the Spikes container.
 */
class Spikes::const_iterator : public boost::iterator_facade<
    const_iterator,
    brion::Spike,
    std::bidirectional_iterator_tag,
    brion::Spike
>
{
    friend class boost::iterator_core_access;
    friend class Spikes;

public:
    /** Create an undefined / invalid iterator. */
    const_iterator();

    /** Copy constructor. */
    const_iterator( const const_iterator& rhs );

    /** Assignment operator. */
    const_iterator& operator= (const const_iterator& rhs);

    /** Destructor. */
    ~const_iterator();

private:
    detail::Spikes_const_iterator* _impl;

    explicit const_iterator( detail::Spikes_const_iterator* impl );
    brion::Spike dereference() const;
    bool equal( const const_iterator &other ) const;
    void increment();
    void decrement();
};

}
#endif
