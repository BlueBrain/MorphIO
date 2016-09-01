
/* Copyright (c) 2006-2016, Juan Hernando <jhernando@fi.upm.es>
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

#include <brain/spikes.h>

namespace bp = boost::python;

namespace brain
{

namespace
{
struct Conversion
{
    static PyObject* convert(const std::pair< float, unsigned int >& pair)
    {
        bp::tuple t = bp::make_tuple(pair.first, pair.second);
        Py_INCREF(t.ptr());
        return t.ptr();
    }
};
}

void export_Spikes()
{

bp::to_python_converter< brion::Spike, Conversion >();

bp::class_< Spikes >( "Spikes", bp::no_init )
    .def( "__iter__", bp::range( &Spikes::begin, &Spikes::end ))
    .def( "__len__", &Spikes::size )
;

}

}
