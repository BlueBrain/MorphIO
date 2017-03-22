/* Copyright (c) 2013-2016, EPFL/Blue Brain Project
 *                          Juan Hernando <juan.hernando@epfl.ch>
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

#include "helpers.h"
#include "arrayHelpers.h"

namespace brain
{
namespace
{
bool _gidsFromIterable(const boost::python::object& iterable, uint32_ts& result)
{
    bool sorted = true;
    try
    {
        result.clear();
        result.reserve(len(iterable));
        // Copying the elements in the iterable to std vector for using
        // insertion
        // from an iterable
        boost::python::stl_input_iterator<unsigned int> i(iterable), end;
        uint32_t last = 0;
        for (; i != end; ++i)
        {
            const uint32_t gid = *i;
            if (last >= gid)
                sorted = false;
            else
                last = gid;
            result.push_back(gid);
        }
    }
    catch (...)
    {
        PyErr_SetString(PyExc_ValueError, "Cannot convert argument to GID set");
        boost::python::throw_error_already_set();
    }
    return sorted;
}
}

GIDSet gidsFromPython(const boost::python::object& object)
{
    uint32_ts vector;

    if (isArray(object))
        gidsFromNumpy(object, vector);
    else
        _gidsFromIterable(object, vector);

    GIDSet gids;
    gids.insert(vector.begin(), vector.end());
    return gids;
}

void gidsFromPython(const boost::python::object& object, GIDSet& result,
                    uint32_ts& mapping)
{
    uint32_ts vector;

    const bool sorted = isArray(object) ? gidsFromNumpy(object, vector)
                                        : _gidsFromIterable(object, vector);

    std::unordered_map<uint32_t, uint32_t> gidToInput;
    if (!sorted)
    {
        gidToInput.reserve(vector.size());
        // Building the GID to input index table
        for (size_t i = 0; i != vector.size(); ++i)
        {
            auto iter = gidToInput.insert(std::make_pair(vector[i], i));
            if (!iter.second)
            {
                PyErr_SetString(PyExc_ValueError, "Repeated GID found");
                boost::python::throw_error_already_set();
            }
        }
    }

    result.clear();
    result.insert(vector.begin(), vector.end());

    mapping.clear();
    if (!sorted)
    {
        mapping.reserve(result.size());
        for (auto gid : result)
            mapping.push_back(gidToInput[gid]);
    }
}
}
