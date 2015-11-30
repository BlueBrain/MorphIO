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

#include "circuit.h"
#include "morphology.h"

#include <brion/blueConfig.h>
#include <brion/circuit.h>
#include <brion/morphology.h>
#include <brion/target.h>

#include <lunchbox/log.h>

#include <boost/foreach.hpp>

namespace brain
{

class Circuit::Impl
{
public:
    Impl( const brion::BlueConfig& config )
        : _circuit( config.getCircuitSource( ))
        , _morphologySource( config.getMorphologySource( ))
        , _circuitTarget( config.getCircuitTarget( ))
        , _targetParsers( config.getTargets( ))
    {
    }

    GIDSet getGIDs( const std::string& target ) const
    {
        return brion::Target::parse(
            _targetParsers, target.empty() ? _circuitTarget : target );
    }

    const brion::Circuit& getCircuit() const
    {
        return _circuit;
    }

    URI getMorphologyURI( const std::string& name ) const
    {
        URI uri;
        uri.setPath( _morphologySource.getPath() + "/" + name + ".h5" );
        uri.setScheme( "file" );
        return uri;
    }

private:
    const brion::Circuit _circuit;
    const brion::URI _morphologySource;
    const std::string _circuitTarget;
    const brion::Targets _targetParsers;

};

Circuit::Circuit( const URI& source )
  : _impl( new Impl( brion::BlueConfig( source.getPath( ))))
{
}

Circuit::Circuit( const brion::BlueConfig& config )
  : _impl( new Impl( config ))
{
}

Circuit::~Circuit()
{
    delete _impl;
}

GIDSet Circuit::getGIDs( const std::string& target ) const
{
    return _impl->getGIDs( target );
}

URIs Circuit::getMorphologyURIs( const GIDSet& gids ) const
{
    const brion::NeuronMatrix& matrix =
        _impl->getCircuit().get( gids, brion::NEURON_MORPHOLOGY_NAME );

    URIs uris;
    uris.reserve( gids.size( ));
    for( size_t index = 0; index != gids.size(); ++index )
        uris.push_back( _impl->getMorphologyURI( matrix[index][0] ));
    return uris;
}

Morphologies Circuit::loadMorphologies( const GIDSet& gids,
                                        Coordinates coords ) const
{
    // Creating all the brion::Morphologies objects first to detect a missing
    // morphology before loading anything.
    const URIs& uris = getMorphologyURIs( gids );
    std::vector< boost::shared_ptr< brion::Morphology > > raw;
    raw.reserve( uris.size( ));
    BOOST_FOREACH( const URI& uri, uris )
        raw.push_back( boost::shared_ptr< brion::Morphology >(
                           new brion::Morphology( uri.getPath( ))));

    Morphologies result;
    result.reserve( gids.size( ));

    if( coords == COORDINATES_GLOBAL )
    {
        const Matrix4fs& transforms = getTransforms( gids );
        for( size_t i = 0; i != uris.size(); ++i )
            result.push_back(
                MorphologyPtr( new Morphology( *raw[i], transforms[i] )));
    }
    else
    {
        std::map< std::string, MorphologyPtr > loaded;
        for( size_t i = 0; i != uris.size(); ++i )
        {
            MorphologyPtr& morphology = loaded[uris[i].getPath()];
            if( !morphology )
                morphology.reset( new Morphology( *raw[i] ));
            result.push_back( morphology );
        }
    }
    return result;
}

Vector3fs Circuit::getPositions( const GIDSet& gids ) const
{
    const brion::NeuronMatrix& data = _impl->getCircuit().get(
        gids, brion::NEURON_POSITION_X | brion::NEURON_POSITION_Y |
             brion::NEURON_POSITION_Z );

    brion::GIDSet::const_iterator gid = gids.begin();
    Vector3fs positions( gids.size( ));
    #pragma omp parallel for
    for( size_t i = 0; i < gids.size(); ++i )
    {
        try
        {
            positions[i] =
                brion::Vector3f( boost::lexical_cast< float >( data[i][0] ),
                                 boost::lexical_cast< float >( data[i][1] ),
                                 boost::lexical_cast< float >( data[i][2] ));
        }
        catch( const boost::bad_lexical_cast& )
        {
            LBWARN << "Error parsing circuit position or orientation for gid "
                   << *gid << ". Morphology not transformed." << std::endl;
            positions[i] = Vector3f::ZERO;
        }
        #pragma omp critical (brain_circuit_getPositions)
        ++gid;
    }
    return positions;
}

Matrix4fs Circuit::getTransforms( const GIDSet& gids ) const
{
    const brion::NeuronMatrix& data = _impl->getCircuit().get(
        gids, brion::NEURON_POSITION_X | brion::NEURON_POSITION_Y |
             brion::NEURON_POSITION_Z | brion::NEURON_ROTATION );

    brion::GIDSet::const_iterator gid = gids.begin();
    Matrix4fs transforms( gids.size(), Matrix4f::IDENTITY );
    #pragma omp parallel for
    for( size_t i = 0; i < gids.size(); ++i )
    {
        Matrix4f& matrix = transforms[i];
        try
        {
            matrix.rotate_y(
                boost::lexical_cast< float >( data[i][3] ) * ( M_PI/180.0f ));
            matrix.set_translation(
                Vector3f( boost::lexical_cast< float >( data[i][0] ),
                          boost::lexical_cast< float >( data[i][1] ),
                          boost::lexical_cast< float >( data[i][2] )));
        }
        catch( const boost::bad_lexical_cast& )
        {
            LBWARN << "Error parsing circuit position or orientation for gid "
                   << *gid << ". Morphology not transformed." << std::endl;
            matrix = Matrix4f::IDENTITY;
        }
        #pragma omp critical (brain_circuit_getTransforms)
        ++gid;
    }
    return transforms;
}

}
