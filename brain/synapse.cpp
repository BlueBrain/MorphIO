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
Synapse::Synapse(const Synapses& synapses, const size_t index)
    : _synapses(synapses)
    , _index(index)
{
}

Synapse::~Synapse()
{
}

SynapseGID Synapse::getGID() const
{
    return std::make_pair(getPostsynapticGID(), _synapses.indices()[_index]);
}

uint32_t Synapse::getPresynapticGID() const
{
    return _synapses.preGIDs()[_index];
}

uint32_t Synapse::getPresynapticSectionID() const
{
    return _synapses.preSectionIDs()[_index];
}

uint32_t Synapse::getPresynapticSegmentID() const
{
    return _synapses.preSegmentIDs()[_index];
}

float Synapse::getPresynapticDistance() const
{
    return _synapses.preDistances()[_index];
}

Vector3f Synapse::getPresynapticSurfacePosition() const
{
    if (!_synapses.preSurfaceXPositions())
        throw std::runtime_error("Surface synapse positions not available");

    return Vector3f(_synapses.preSurfaceXPositions()[_index],
                    _synapses.preSurfaceYPositions()[_index],
                    _synapses.preSurfaceZPositions()[_index]);
}

Vector3f Synapse::getPresynapticCenterPosition() const
{
    return Vector3f(_synapses.preCenterXPositions()[_index],
                    _synapses.preCenterYPositions()[_index],
                    _synapses.preCenterZPositions()[_index]);
}

uint32_t Synapse::getPostsynapticGID() const
{
    return _synapses.postGIDs()[_index];
}

uint32_t Synapse::getPostsynapticSectionID() const
{
    return _synapses.postSectionIDs()[_index];
}

uint32_t Synapse::getPostsynapticSegmentID() const
{
    return _synapses.postSegmentIDs()[_index];
}

float Synapse::getPostsynapticDistance() const
{
    return _synapses.postDistances()[_index];
}

Vector3f Synapse::getPostsynapticSurfacePosition() const
{
    if (!_synapses.postSurfaceXPositions())
        throw std::runtime_error("Surface synapse positions not available");

    return Vector3f(_synapses.postSurfaceXPositions()[_index],
                    _synapses.postSurfaceYPositions()[_index],
                    _synapses.postSurfaceZPositions()[_index]);
}

Vector3f Synapse::getPostsynapticCenterPosition() const
{
    return Vector3f(_synapses.postCenterXPositions()[_index],
                    _synapses.postCenterYPositions()[_index],
                    _synapses.postCenterZPositions()[_index]);
}

float Synapse::getDelay() const
{
    return _synapses.delays()[_index];
}

float Synapse::getConductance() const
{
    return _synapses.conductances()[_index];
}

float Synapse::getUtilization() const
{
    return _synapses.utilizations()[_index];
}

float Synapse::getDepression() const
{
    return _synapses.depressions()[_index];
}

float Synapse::getFacilitation() const
{
    return _synapses.facilitations()[_index];
}

float Synapse::getDecay() const
{
    return _synapses.decays()[_index];
}

int Synapse::getEfficacy() const
{
    return _synapses.efficacies()[_index];
}
}
