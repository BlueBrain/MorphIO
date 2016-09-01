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

#ifndef BRAIN_PYTHON_HELPERS_H
#define BRAIN_PYTHON_HELPERS_H

#include <boost/python.hpp>
#include <boost/python/stl_iterator.hpp>

#include <brain/types.h>

namespace brain
{

template< typename T >
inline std::vector< T > vectorFromPython( boost::python::object o,
                                          const char* errorMessage )
{
    std::vector< T > vector;
    try
    {
        vector.reserve( boost::python::len( o ));
        boost::python::stl_input_iterator< T > i( o ), end;
        for( ; i != end; ++i)
            vector.push_back( *i );
    }
    catch(...)
    {
        PyErr_SetString(PyExc_ValueError, errorMessage );
        boost::python::throw_error_already_set();
    }
    return vector;
}

template< typename T >
inline boost::python::list toPythonList( const std::vector< T >& vector )
{
    boost::python::list result;
    for( const T& i  : vector )
        result.append( i );
    return result;
}

inline boost::python::object toPythonSet( const GIDSet& ids )
{
    boost::python::object set( boost::python::handle<>( PySet_New( 0 )));
    for( uint32_t gid : ids )
#if PY_MAJOR_VERSION >= 3
        PySet_Add( set.ptr(), PyLong_FromLong( gid ));
#else
        PySet_Add( set.ptr(), PyInt_FromLong( gid ));
#endif

    return set;
}

inline GIDSet gidsFromPython( const boost::python::object iterable )
{
    GIDSet result;
    try
    {
        boost::python::stl_input_iterator< unsigned int > i( iterable ), end;
        uint32_ts vector;
        vector.reserve( len( iterable ));
        for( ; i != end; ++i)
            vector.push_back( *i );
        result.insert( vector.begin(), vector.end( ));
    }
    catch(...)
    {
        PyErr_SetString( PyExc_ValueError,
                         "Cannot convert argument to GID set" );
        boost::python::throw_error_already_set();
    }
    return result;
}

}
#endif
