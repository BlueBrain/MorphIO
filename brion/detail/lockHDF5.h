/* Copyright (c) 2013-2017, EPFL/Blue Brain Project
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

#ifndef BRION_DETAIL_LOCKHDF5
#define BRION_DETAIL_LOCKHDF5

#include <mutex>

namespace brion
{
namespace detail
{
// Every access to hdf5 must be serialized if HDF5 does not take care of it
// which needs a thread-safe built of the library.
// http://www.hdfgroup.org/hdf5-quest.html#gconc
inline std::mutex* hdf5Lock()
{
    static std::mutex _hdf5Lock;
    return &_hdf5Lock;
}
}
}

#endif
