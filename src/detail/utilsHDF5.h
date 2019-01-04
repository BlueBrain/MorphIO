/* Copyright (c) 2016-2017, EPFL/Blue Brain Project
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

#ifndef BRAIN_DETAIL_UTILSHDF5
#define BRAIN_DETAIL_UTILSHDF5

#include <highfive/H5Attribute.hpp>
#include <highfive/H5DataType.hpp>

#include <morphio/types.h>
namespace HighFive {
template <>
inline AtomicType<morphio::SectionType>::AtomicType()
{
    _hid = H5Tcopy(H5T_NATIVE_INT);
}

template <>
inline AtomicType<morphio::MorphologyVersion>::AtomicType()
{
    _hid = H5Tcopy(H5T_NATIVE_INT);
}

template <>
inline AtomicType<std::array<int, 2>>::AtomicType()
{
    _hid = H5Tcopy(H5T_NATIVE_INT);
}

template <>
inline AtomicType<morphio::Point>::AtomicType()
{
    _hid = H5Tcopy(H5T_NATIVE_INT);
}
} // namespace HighFive

namespace morphio {
namespace detail {
/** Add a string attribute to the given H5 object. */
template <typename T>
inline void addStringAttribute(HighFive::AnnotateTraits<T>& object,
    const std::string& name,
    const std::string& value)
{
    auto dataspace = HighFive::DataSpace::From(value);
    auto attr = object.createAttribute(name, dataspace,
        HighFive::AtomicType<std::string>());
    attr.write(value);
}
} // namespace detail
} // namespace morphio

#endif
