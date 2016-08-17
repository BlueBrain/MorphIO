/* Copyright (c) 2013-2016, EPFL/Blue Brain Project
 *                          Daniel.Nachbaur@epfl.ch
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


#include "synapse.h"
#include "synapses.h"

#include <vmmlib/vector.hpp>

namespace brain
{

Synapse::Synapse( const Synapses& synapses, const size_t index )
    : _synapses( synapses )
    , _index( index )
{
}

Synapse::~Synapse()
{
}

SynapseGID Synapse::getGID() const
{
    return std::make_pair( getPostsynapticGID(),
                           _synapses.index()[_index] );
}

uint32_t Synapse::getPresynapticGID() const
{
    return _synapses.preGID()[_index];
}

uint32_t Synapse::getPresynapticSectionID() const
{
    return _synapses.preSectionID()[_index];
}

uint32_t Synapse::getPresynapticSegmentID() const
{
    return _synapses.preSegmentID()[_index];
}

float Synapse::getPresynapticDistance() const
{
    return _synapses.preDistance()[_index];
}

Vector3f Synapse::getPresynapticSurfacePosition() const
{
    return Vector3f( _synapses.preSurfacePositionX()[_index],
                     _synapses.preSurfacePositionY()[_index],
                     _synapses.preSurfacePositionZ()[_index] );
}

Vector3f Synapse::getPresynapticCenterPosition() const
{
    return Vector3f( _synapses.preCenterPositionX()[_index],
                     _synapses.preCenterPositionY()[_index],
                     _synapses.preCenterPositionZ()[_index] );
}

uint32_t Synapse::getPostsynapticGID() const
{
    return _synapses.postGID()[_index];
}

uint32_t Synapse::getPostsynapticSectionID() const
{
    return _synapses.postSectionID()[_index];
}

uint32_t Synapse::getPostsynapticSegmentID() const
{
    return _synapses.postSegmentID()[_index];
}

float Synapse::getPostsynapticDistance() const
{
    return _synapses.postDistance()[_index];
}

Vector3f Synapse::getPostsynapticSurfacePosition() const
{
    return Vector3f( _synapses.postSurfacePositionX()[_index],
                     _synapses.postSurfacePositionY()[_index],
                     _synapses.postSurfacePositionZ()[_index] );
}

Vector3f Synapse::getPostsynapticCenterPosition() const
{
    return Vector3f( _synapses.postCenterPositionX()[_index],
                     _synapses.postCenterPositionY()[_index],
                     _synapses.postCenterPositionZ()[_index] );
}

float Synapse::getDelay() const
{
    return _synapses.delay()[_index];
}

float Synapse::getConductance() const
{
    return _synapses.conductance()[_index];
}

float Synapse::getUtilization() const
{
    return _synapses.utilization()[_index];
}

float Synapse::getDepression() const
{
    return _synapses.depression()[_index];
}

float Synapse::getFacilitation() const
{
    return _synapses.facilitation()[_index];
}

float Synapse::getDecay() const
{
    return _synapses.decay()[_index];
}

int Synapse::getEfficacy() const
{
    return _synapses.efficacy()[_index];
}

}
