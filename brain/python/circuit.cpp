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

#include <boost/python.hpp>
#include <boost/python/stl_iterator.hpp>

#include "arrayHelpers.h"
#include "helpers.h"
#include "types.h"

#include <brain/circuit.h>
#include <brain/synapses.h>
#include <brain/synapsesStream.h>

namespace bp = boost::python;

namespace brain
{
namespace
{

CircuitPtr Circuit_initFromURI( const std::string& uri )
{
    return CircuitPtr( new Circuit( URI( uri )));
}

bp::object Circuit_getAllGIDs( const Circuit& circuit )
{
    return toPythonSet( circuit.getGIDs( ));
}

bp::object Circuit_getGIDs( const Circuit& circuit, const std::string& target )
{
    return toPythonSet( circuit.getGIDs( target ));
}

bp::object Circuit_getRandomTargetGIDs(
    const Circuit& circuit, const float fraction, const std::string& target )
{
    return toPythonSet( circuit.getRandomGIDs( fraction, target ));
}

bp::object Circuit_getRandomGIDs( const Circuit& circuit, const float fraction )
{
    return toPythonSet( circuit.getRandomGIDs( fraction ));
}

#define GET_CIRCUIT_PROPERTY_FOR_GIDS(property) \
    bp::object Circuit_get##property( const Circuit& circuit, bp::object gids ) \
{                                                                               \
    return toNumpy( circuit.get##property( gidsFromPython( gids )));            \
}

GET_CIRCUIT_PROPERTY_FOR_GIDS(MorphologyTypes)
GET_CIRCUIT_PROPERTY_FOR_GIDS(ElectrophysiologyTypes)

bp::object Circuit_getMorphologyURIs( const Circuit& circuit, bp::object gids )
{
    return toPythonList( circuit.getMorphologyURIs( gidsFromPython( gids )));
}

#define GET_CIRCUIT_PROPERTY_VALUES(property) \
    bp::list Circuit_get##property( const Circuit& circuit )\
{                                                           \
    return toPythonList( circuit.get##property( ));         \
}
GET_CIRCUIT_PROPERTY_VALUES(MorphologyNames)
GET_CIRCUIT_PROPERTY_VALUES(ElectrophysiologyNames)

bp::list Circuit_loadMorphologies( const Circuit& circuit,
                                   bp::object gids, Circuit::Coordinates coords )
{
    return toPythonList(
        circuit.loadMorphologies( gidsFromPython( gids ), coords ));
}

bp::object Circuit_getPositions( const Circuit& circuit, bp::object gids )
{
    return toNumpy( circuit.getPositions( gidsFromPython( gids )));
}

bp::object Circuit_getTransforms( const Circuit& circuit, bp::object gids )
{
    bp::object matrices =
        toNumpy( circuit.getTransforms( gidsFromPython( gids )));
    // We want the result to be indexed using regular mathematical notation
    // even if the actual storage is column-major.
    return matrices.attr( "transpose" )( 0, 2, 1 );
}

bp::object Circuit_getRotations( const Circuit& circuit, bp::object gids )
{
    return toNumpy( circuit.getRotations( gidsFromPython( gids )));
}

SynapsesWrapper Circuit_getAfferentSynapses(
    const CircuitPtr& circuit, bp::object gids,
    const brain::SynapsePrefetch prefetch )
{
    return SynapsesWrapper( circuit->getAfferentSynapses( gidsFromPython( gids ),
                                                          prefetch ),
                            circuit );
}

SynapsesWrapper Circuit_getEfferentSynapses(
    const CircuitPtr& circuit, bp::object gids,
    const brain::SynapsePrefetch prefetch )
{
    return SynapsesWrapper( circuit->getEfferentSynapses( gidsFromPython( gids ),
                                                          prefetch ),
                            circuit );
}

SynapsesWrapper Circuit_getProjectedSynapses(
    const CircuitPtr& circuit, bp::object pre, bp::object post,
    const brain::SynapsePrefetch prefetch )
{
    return SynapsesWrapper( circuit->getProjectedSynapses(
                                gidsFromPython( pre ), gidsFromPython( post ),
                                prefetch ),
                            circuit );
}

}

void export_Circuit()
{

bp::class_< Circuit, boost::noncopyable, CircuitPtr >
    circuitWrapper( "Circuit", bp::no_init );

bp::scope circuitScope = circuitWrapper;

bp::enum_< Circuit::Coordinates >( "Coordinates" )
    .value( "global_", Circuit::Coordinates::global )
    .value( "local", Circuit::Coordinates::local )
    ;

circuitWrapper
    .def( "__init__", bp::make_constructor( Circuit_initFromURI ))
    .def( "gids", Circuit_getAllGIDs )
    .def( "gids", Circuit_getGIDs )
    .def( "random_gids", Circuit_getRandomTargetGIDs )
    .def( "random_gids", Circuit_getRandomGIDs )
    .def( "morphology_uris", Circuit_getMorphologyURIs )
    .def( "load_morphologies", Circuit_loadMorphologies )
    .def( "positions", Circuit_getPositions )
    .def( "morphology_types", Circuit_getMorphologyTypes )
    .def( "morphology_names", Circuit_getMorphologyNames )
    .def( "electrophysiology_types", Circuit_getElectrophysiologyTypes )
    .def( "electrophysiology_names", Circuit_getElectrophysiologyNames )
    .def( "transforms", Circuit_getTransforms )
    .def( "rotations", Circuit_getRotations )
    .def( "num_neurons", &Circuit::getNumNeurons )
    .def( "afferent_synapses", Circuit_getAfferentSynapses,
          ( bp::arg( "gids"), bp::arg( "prefetch" ) = SynapsePrefetch::none ))
    .def( "efferent_synapses", Circuit_getEfferentSynapses,
          ( bp::arg( "gids"), bp::arg( "prefetch" ) = SynapsePrefetch::none ))
    .def( "projected_synapses", Circuit_getProjectedSynapses,
          ( bp::arg( "preGids"), bp::arg( "postGIDs" ),
            bp::arg( "prefetch" ) = SynapsePrefetch::none ))
    ;
}

}
