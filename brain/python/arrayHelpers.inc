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

#include <utility>

namespace brain
{

/** @internal */
class AbstractCustodian
{
public:
    virtual ~AbstractCustodian() {}
};

/** @internal */
template< typename T >
class Custodian : public AbstractCustodian
{
public:
    Custodian( const T& custodian ) : _custodian( custodian ) {}
private:
    const T _custodian;
};

/** @internal */
template< typename T >
using VectorPtr = std::shared_ptr< std::vector< T >>;

// An boost::shared_ptr is used instead std::shared_ptr because it gets on
// well with Boost.Python.
typedef boost::shared_ptr< AbstractCustodian > AbstractCustodianPtr;

template< typename T >
inline boost::python::object toNumpy( std::vector< T >&& vector )
{
    return boost::python::object(
        VectorPtr< T >( new std::vector< T >( std::move( vector ))));
}

template< typename T >
struct ConstVectorWithCustodian
{
    ConstVectorWithCustodian( const std::vector< T >& v,
                              const AbstractCustodianPtr& c )
        : vector( v )
        , custodian( c )
    {}

    const std::vector< T >& vector;
    AbstractCustodianPtr custodian;
};

template< typename T, typename U >
inline boost::python::object toNumpy( const std::vector< T >& vector,
                                      const U& custodian )
{
    AbstractCustodianPtr custodianProxy( new Custodian< U >( custodian ));
    return boost::python::object(
        ConstVectorWithCustodian< T >( vector, custodianProxy ));
}

template< typename T >
struct ConstArrayWithCustodian
{
    ConstArrayWithCustodian( const T* v, const size_t s,
                             const AbstractCustodianPtr& c )
        : array( v )
        , size( s )
        , custodian( c )
    {}

    const T* array;
    size_t size;
    AbstractCustodianPtr custodian;
};

template< typename T, typename U >
inline boost::python::object toNumpy( const T* array, const size_t size,
                                      const U& custodian )
{
    AbstractCustodianPtr custodianProxy( new Custodian< U >( custodian ));
    return boost::python::object(
        ConstArrayWithCustodian< T >( array, size, custodianProxy ));
}

}
