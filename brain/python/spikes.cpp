
/* Copyright (c) 2006-2017, Juan Hernando <jhernando@fi.upm.es>
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
#include <boost/python.hpp>

#include <brion/types.h>

namespace bp = boost::python;

namespace brain
{
namespace
{
struct Conversion
{
    static PyObject* convert(const brion::Spike& spike)
    {
        bp::tuple t = bp::make_tuple(spike.first, spike.second);
        Py_INCREF(t.ptr());
        return t.ptr();
    }
};
}

// clang-format off
void export_Spikes()
{
    bp::to_python_converter< brion::Spike, Conversion >();

    bp::class_< brion::Spikes >( "Spikes", bp::no_init )
        .def( "__iter__", bp::iterator<brion::Spikes>( ))
        .def( "__len__", &brion::Spikes::size )
    ;
}
// clang-format on
}
