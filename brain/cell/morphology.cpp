/* Copyright (c) 2013-2015, EPFL/Blue Brain Project
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

#include "morphology.h"
#include "section.h"

#include "morphologyImpl.h"

#include <brion/morphology.h>

#include <lunchbox/log.h>

#include <boost/foreach.hpp>

namespace brain
{
namespace cell
{

Morphology::Morphology( const URI& source, const Matrix4f& transform )
    : _impl( new Impl( brion::Morphology( source.getPath( ))))
{
    _impl->ref();
    _impl->transform( transform );
}

Morphology::Morphology( const brion::Morphology& morphology,
                        const Matrix4f& transform )
    : _impl( new Impl( morphology ))
{
    _impl->ref();
    _impl->transform( transform );
}

Morphology::Morphology( const URI& source )
    : _impl( new Impl( brion::Morphology( source.getPath( ))))
{
    _impl->ref();
}

Morphology::Morphology( const brion::Morphology& morphology )
    : _impl( new Impl( morphology ))
{
    _impl->ref();
}

Morphology::~Morphology()
{
    _impl->unref();
}

const Vector4fs& Morphology::getPoints() const
{
    return *_impl->points;
}

const Vector2is& Morphology::getSections() const
{
    return *_impl->sections;
}

const SectionTypes& Morphology::getSectionTypes() const
{
    return *_impl->types;
}

const Vector2is& Morphology::getApicals() const
{
    return *_impl->apicals;
}

uint32_ts Morphology::getSectionIDs( const SectionTypes& types ) const
{
    return _impl->getSectionIDs( types );
}

Sections Morphology::getSections( const SectionType type ) const
{
    const SectionTypes types( 1, type );
    const uint32_ts ids = _impl->getSectionIDs( types );
    Sections result;
    BOOST_FOREACH( uint32_t id, ids )
        result.push_back( Section( id, _impl ));
    return result;
}

Sections Morphology::getSections( const SectionTypes& types ) const
{
    uint32_ts ids = _impl->getSectionIDs( types );
    Sections result;
    BOOST_FOREACH( uint32_t id, ids )
        result.push_back( Section( id, _impl ));
    return result;
}

Section Morphology::getSection( const uint32_t& id ) const
{
    if( _impl->sections->size() <= id )
    {
        std::stringstream msg;
        msg << "Section ID out of range: " << id;
        LBTHROW( std::runtime_error( msg.str( )));
    }

    return Section( id, _impl );
}

}
}
