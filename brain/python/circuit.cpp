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
#include "docstrings.h"

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
    circuitWrapper( "Circuit", DOXY_CLASS( brain::Circuit ), bp::no_init );

bp::scope circuitScope = circuitWrapper;

bp::enum_< Circuit::Coordinates >( "Coordinates" )
    .value( "global_", Circuit::Coordinates::global )
    .value( "local", Circuit::Coordinates::local )
    ;

const auto selfarg = bp::arg( "self" );

// Do not modify whitespace on DOXY_FN lines

circuitWrapper
    .def( "__init__", bp::make_constructor( Circuit_initFromURI ),
          DOXY_FN( brain::Circuit::Circuit(const URI&)))
    .def( "gids", Circuit_getAllGIDs, ( selfarg ),
          DOXY_FN( brain::Circuit::getGIDs() const))
    .def( "gids", Circuit_getGIDs, ( selfarg, bp::arg( "target" )),
          DOXY_FN( brain::Circuit::getGIDs(const std::string&) const))
    .def( "random_gids", Circuit_getRandomTargetGIDs,
          ( selfarg, bp::arg( "fraction" ), bp::arg( "target" )),
          DOXY_FN( brain::Circuit::getRandomGIDs(float, const std::string&) const))
    .def( "random_gids", Circuit_getRandomGIDs,
          ( selfarg, bp::arg( "fraction" )),
          DOXY_FN( brain::Circuit::getRandomGIDs(float) const))
    .def( "morphology_uris", Circuit_getMorphologyURIs,
          ( selfarg, bp::arg( "gids" )),
          DOXY_FN( brain::Circuit::getMorphologyURIs ))
    .def( "load_morphologies", Circuit_loadMorphologies,
          ( selfarg, bp::arg( "gids" ), bp::arg( "coords" )),
          DOXY_FN( brain::Circuit::loadMorphologies ))
    .def( "positions", Circuit_getPositions, ( selfarg, bp::arg( "gids" )),
          DOXY_FN( brain::Circuit::getPositions ))
    .def( "morphology_types", Circuit_getMorphologyTypes,
          ( selfarg, bp::arg( "gids" )),
          DOXY_FN( brain::Circuit::getMorphologyTypes ))
    .def( "morphology_names", Circuit_getMorphologyNames, ( selfarg ),
          DOXY_FN( brain::Circuit::getMorphologyNames ))
    .def( "electrophysiology_types", Circuit_getElectrophysiologyTypes,
          ( selfarg, bp::arg( "gids" )),
          DOXY_FN( brain::Circuit::getElectrophysiologyTypes ))
    .def( "electrophysiology_names", Circuit_getElectrophysiologyNames,
          ( selfarg),
          DOXY_FN( brain::Circuit::getElectrophysiologyNames ))
    .def( "transforms", Circuit_getTransforms,
          ( selfarg, bp::arg( "gids" )),
          DOXY_FN( brain::Circuit::getTransforms ))
    .def( "rotations", Circuit_getRotations,
          ( selfarg, bp::arg( "gids" )),
          DOXY_FN( brain::Circuit::getRotations ))
    .def( "num_neurons", &Circuit::getNumNeurons, ( selfarg ),
          DOXY_FN( brain::Circuit::getNumNeurons ))
    .def( "afferent_synapses", Circuit_getAfferentSynapses,
          ( selfarg, bp::arg( "gids"),
            bp::arg( "prefetch" ) = SynapsePrefetch::none ),
          DOXY_FN( brain::Circuit::getAfferentSynapses ))
    .def( "efferent_synapses", Circuit_getEfferentSynapses,
          ( selfarg, bp::arg( "gids"),
            bp::arg( "prefetch" ) = SynapsePrefetch::none ),
          DOXY_FN( brain::Circuit::getEfferentSynapses ))
    .def( "projected_synapses", Circuit_getProjectedSynapses,
          ( selfarg, bp::arg( "preGids"), bp::arg( "postGIDs" ),
            bp::arg( "prefetch" ) = SynapsePrefetch::none ),
          DOXY_FN( brain::Circuit::getProjectedSynapses ))
    ;
}

}
