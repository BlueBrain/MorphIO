
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

#include <brion/morphology.h>
#include <brion/circuit.h>

#include <lunchbox/log.h>

#include <boost/foreach.hpp>

#include <bitset>

namespace brain
{

class Morphology::Impl
{
public:
    const brion::Vector4fsPtr points;
    const brion::Vector2isPtr sections;
    const brion::SectionTypesPtr types;
    const brion::Vector2isPtr apicals;
    Vector3f somaPosition;

    Impl( const brion::Morphology& morphology )
        : points( morphology.readPoints( brion::MORPHOLOGY_UNDEFINED ))
        , sections( morphology.readSections( brion::MORPHOLOGY_UNDEFINED ))
        , types( morphology.readSectionTypes( ))
        , apicals( morphology.readApicals( ))
        , somaPosition( Vector3f::ZERO ) // the soma is assumed to be
                                         // centered at the 0, 0, 0
    {
    }

    uint32_ts getSectionIDs( const SectionTypes& requestedTypes ) const
    {
        std::bitset< brion::SECTION_APICAL_DENDRITE > bits;
        BOOST_FOREACH( const SectionType type, requestedTypes )
            bits[size_t( type )] = true;

        uint32_ts result;
        for( size_t i = 0; i != types->size(); ++i )
        {
            const SectionType type = ( *types )[i];
            if( type == brion::SECTION_ALL )
                LBWARN << "Unknown section type " << int(type) << std::endl;
            else
                if( bits[size_t( type )] )
                    result.push_back( i );
        }
        return result;
    }

    Vector4fs getSectionSamples(
        const size_t sectionID, const floats& samplePoints ) const
    {
        // If the section is the soma return directly the soma position.
        if(( *types )[sectionID] == brion::SECTION_SOMA )
        {
            Vector4fs result;
            for( size_t i = 0; i != samplePoints.size(); ++i )
                result.push_back( somaPosition );
            return result;
        }

        const size_t start = ( *sections )[sectionID][0];
        const size_t end = sectionID == sections->size() - 1 ?
            points->size() : ( *sections )[sectionID + 1][0];

        Vector4fs result;

        if( end <= start )
        {
            LBWARN << "Trying to sample broken morphology or empty section "
                   << sectionID << std::endl;
            return result;
        }

        result.reserve( samplePoints.size( ));

        // Dealing with the degenerate case of single point sections.
        if( start + 1 == end )
        {
            for( size_t i = 0; i != samplePoints.size(); ++i)
                result.push_back(( *points )[start] );
            return result;
        }

        // Computing the accumulated length at each segment of the section.
        floats accumLengths;
        accumLengths.reserve( end - start );
        accumLengths.push_back(0);
        for( size_t i = start; i != end - 1; ++i )
        {
            const Vector4f& segmentStart = ( *points )[i];
            const Vector4f& segmentEnd = ( *points )[i + 1];
            accumLengths.push_back( accumLengths.back() +
                                    ( segmentEnd - segmentStart ).length( ));
        }
        const float totalLength = accumLengths.back();

        BOOST_FOREACH( const float point, samplePoints )
        {
            // Finding the segment index for the requested sampling position.
            const float length =
                std::max( 0.f, std::min( 1.f, point )) * totalLength;
            size_t index = 0;
            for ( ; accumLengths[index + 1] < length &&
                    index < accumLengths.size() - 1; ++index )
                ;

            // Interpolating the cross section at point.
            const float alpha = ( length - accumLengths[index] ) /
                              ( accumLengths[index + 1] - accumLengths[index] );
            const Vector4f sample = ( *points )[start + index + 1] * alpha +
                                    ( *points )[start + index] * (1 - alpha );
            result.push_back( sample );
        }

        return result;
    }

    void transform( const Matrix4f& transformation )
    {
        #pragma omp parallel for
        for( size_t i = 0; i < points->size(); ++i)
        {
            Vector4f& p = ( *points )[i];
            const Vector3f pp = transformation * p.get_sub_vector< 3 >();
            p.get_sub_vector< 3 >() = pp;
        }
        somaPosition = transformation * somaPosition;
    }
};

Morphology::Morphology( const URI& source, const Matrix4f& transform )
    : _impl( new Impl( brion::Morphology( source.getPath( ))))
{
    _impl->transform( transform );
}

Morphology::Morphology( const brion::Morphology& morphology,
                        const Matrix4f& transform )
    : _impl( new Impl( morphology ))
{
    _impl->transform( transform );
}

Morphology::Morphology( const URI& source )
    : _impl( new Impl( brion::Morphology( source.getPath( ))))
{
}

Morphology::Morphology( const brion::Morphology& morphology )
    : _impl( new Impl( morphology ))
{
}

Morphology::~Morphology()
{
    delete _impl;
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

Vector4fs Morphology::getSectionSamples(
    const size_t sectionID, const floats& points ) const
{
    return _impl->getSectionSamples( sectionID, points );
}

}

