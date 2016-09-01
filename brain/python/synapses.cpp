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

#include "types.h"
#include "arrayHelpers.h"

#include <brain/synapses.h>
#include <brain/synapses.h>

#include <vmmlib/vector.hpp>

namespace bp = boost::python;

namespace brain
{

namespace
{

bool nonzero( const SynapsesWrapper& )
{
    return true;
}

SynapseWrapper Synapses_get( const SynapsesWrapper& synapses, long int index )
{
    if( index < 0 )
        index = synapses.size() + index;
    if( index < 0 || size_t(index) >= synapses.size( ))
    {
        PyErr_SetString( PyExc_IndexError, "Index out of bounds" );
        bp::throw_error_already_set();
    }
    return SynapseWrapper( synapses[index], synapses, synapses._circuit );
}

#define GET_SYNAPSES_ARRAY_PROPERTY( type, name ) \
bp::object Synapses_##name( const SynapsesWrapper& synapses )       \
    { return toNumpy( synapses.name(), synapses.size(), synapses._impl ); }

GET_SYNAPSES_ARRAY_PROPERTY( size_t, indices )
GET_SYNAPSES_ARRAY_PROPERTY( uin32_t, preGIDs )
GET_SYNAPSES_ARRAY_PROPERTY( uin32_t, preSectionIDs )
GET_SYNAPSES_ARRAY_PROPERTY( uin32_t, preSegmentIDs )
GET_SYNAPSES_ARRAY_PROPERTY( float, preDistances )
GET_SYNAPSES_ARRAY_PROPERTY( float, preSurfaceXPositions )
GET_SYNAPSES_ARRAY_PROPERTY( float, preSurfaceYPositions )
GET_SYNAPSES_ARRAY_PROPERTY( float, preSurfaceZPositions )
GET_SYNAPSES_ARRAY_PROPERTY( float, preCenterXPositions )
GET_SYNAPSES_ARRAY_PROPERTY( float, preCenterYPositions )
GET_SYNAPSES_ARRAY_PROPERTY( float, preCenterZPositions )
GET_SYNAPSES_ARRAY_PROPERTY( uint32_t, postGIDs )
GET_SYNAPSES_ARRAY_PROPERTY( uint32_t, postSectionIDs )
GET_SYNAPSES_ARRAY_PROPERTY( uint32_t, postSegmentIDs )
GET_SYNAPSES_ARRAY_PROPERTY( float, postDistances )
GET_SYNAPSES_ARRAY_PROPERTY( float, postSurfaceXPositions )
GET_SYNAPSES_ARRAY_PROPERTY( float, postSurfaceYPositions )
GET_SYNAPSES_ARRAY_PROPERTY( float, postSurfaceZPositions )
GET_SYNAPSES_ARRAY_PROPERTY( float, postCenterXPositions )
GET_SYNAPSES_ARRAY_PROPERTY( float, postCenterYPositions )
GET_SYNAPSES_ARRAY_PROPERTY( float, postCenterZPositions )
GET_SYNAPSES_ARRAY_PROPERTY( float, delays )
GET_SYNAPSES_ARRAY_PROPERTY( float, conductances )
GET_SYNAPSES_ARRAY_PROPERTY( float, utilizations )
GET_SYNAPSES_ARRAY_PROPERTY( float, depressions )
GET_SYNAPSES_ARRAY_PROPERTY( float, facilitations )
GET_SYNAPSES_ARRAY_PROPERTY( float, decays )
GET_SYNAPSES_ARRAY_PROPERTY( int, efficacies )

}

void export_Synapses()
{

bp::class_< SynapseWrapper >( "Synapse", bp::no_init )
    .def( "pre_gid", &Synapse::getPresynapticGID )
    .def( "pre_section", &Synapse::getPresynapticSectionID )
    .def( "pre_segment", &Synapse::getPresynapticSegmentID )
    .def( "pre_distance", &Synapse::getPresynapticDistance )
    .def( "pre_center_position", &Synapse::getPresynapticCenterPosition )
    .def( "pre_surface_position", &Synapse::getPresynapticSurfacePosition )
    .def( "post_gid", &Synapse::getPostsynapticGID )
    .def( "post_section", &Synapse::getPostsynapticSectionID )
    .def( "post_segment", &Synapse::getPostsynapticSegmentID )
    .def( "post_distance", &Synapse::getPostsynapticDistance )
    .def( "post_center_position", &Synapse::getPostsynapticCenterPosition )
    .def( "post_surface_position", &Synapse::getPostsynapticSurfacePosition )
    .def( "delay", &Synapse::getDelay )
    .def( "conductance", &Synapse::getConductance )
    .def( "utilization", &Synapse::getUtilization )
    .def( "depression", &Synapse::getDepression )
    .def( "facilitation", &Synapse::getFacilitation )
    .def( "decay", &Synapse::getDecay )
    .def( "efficacy", &Synapse::getEfficacy )
    ;

bp::class_< SynapsesWrapper >( "Synapses", bp::no_init )
    .def( "__nonzero__", nonzero )
    .def( "__len__", &Synapses::size )
    .def( "__getitem__", Synapses_get )
    // There is no need to wrap the iterator, Python provides one out of the
    // box thanks to __len__ and __getitem__
    .def( "empty", &Synapses::empty )
    .def( "indices", Synapses_indices )
    .def( "pre_gids", Synapses_preGIDs )
    .def( "pre_section_ids", Synapses_preSectionIDs )
    .def( "pre_segment_ids", Synapses_preSegmentIDs )
    .def( "pre_distances", Synapses_preDistances )
    .def( "pre_surface_x_positions", Synapses_preSurfaceXPositions )
    .def( "pre_surface_y_positions", Synapses_preSurfaceYPositions )
    .def( "pre_surface_z_positions", Synapses_preSurfaceZPositions )
    .def( "pre_center_x_positions", Synapses_preCenterXPositions )
    .def( "pre_center_y_positions", Synapses_preCenterYPositions )
    .def( "pre_center_z_positions", Synapses_preCenterZPositions )
    .def( "post_gids", Synapses_postGIDs )
    .def( "post_section_ids", Synapses_postSectionIDs )
    .def( "post_segment_ids", Synapses_postSegmentIDs )
    .def( "post_distances", Synapses_postDistances )
    .def( "post_surface_x_positions", Synapses_postSurfaceXPositions )
    .def( "post_surface_y_positions", Synapses_postSurfaceYPositions )
    .def( "post_surface_z_positions", Synapses_postSurfaceZPositions )
    .def( "post_center_x_positions", Synapses_postCenterXPositions )
    .def( "post_center_y_positions", Synapses_postCenterYPositions )
    .def( "post_center_z_positions", Synapses_postCenterZPositions )
    .def( "delays", Synapses_delays )
    .def( "conductances", Synapses_conductances )
    .def( "utilizations", Synapses_utilizations )
    .def( "depressions", Synapses_depressions )
    .def( "facilitations", Synapses_facilitations )
    .def( "decays", Synapses_decays )
    .def( "efficacies", Synapses_efficacies )
    ;

}

}
