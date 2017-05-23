/* Copyright (c) 2013-2016, Juan Hernando <juan.hernando@epfl.ch>
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

#ifndef BRAIN_PYTHON_ARRAYHELPERS_H
#define BRAIN_PYTHON_ARRAYHELPERS_H

#include <brain/types.h>

#include <boost/python.hpp>
#include <boost/shared_ptr.hpp>
#include <vector>

namespace brain
{
void importArray();

/** @return True if the python object is a numpy.ndarray, false otherwise. */
bool isArray(const boost::python::object& object);

template <typename T>
boost::python::object toNumpy(std::vector<T>&& vector);

/** The custodian object must be copy constructible. In general it's expected
 *   to be a std::shared_ptr or boost::shared_ptr.
 */
template <typename T, typename U>
boost::python::object toNumpy(const std::vector<T>& vector, const U& custodian);

/** The custodian object must be copy constructible. In general it's expected
 *  to be a std::shared_ptr or boost::shared_ptr.
 */
template <typename T, typename U>
boost::python::object toNumpy(const T* array, size_t size, const U& custodian);

boost::python::object toNumpy(const Matrix4f& matrix);

/** Copy the GIDs from a numpy array into a uint32_t vector.
 *  @param object The python object representing the numpy array.
 *  @param result The destination array.
 *  @return true if the GIDs are in ascending order
 */
bool gidsFromNumpy(const boost::python::object& object, uint32_ts& result);

std::pair<const Spike*, size_t> spikesFromNumpy(
    const boost::python::object& object);

/// tuple(timestamp : double, data : 1D ndarray )
boost::python::object frameToTuple(brion::Frame&& frame);

/// tuple(timestamps : 1D ndarray, data : 2D ndarray )
/// or tuple(timestamp : double, data : 1D ndarray) if frames.size == 1
boost::python::object framesToTuple(brion::Frames&& frames);

// Implemented for: Matrix4f
template <typename T>
T fromNumpy(const boost::python::object& object);
}

#include "arrayHelpers.inc"
#endif
