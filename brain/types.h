/* Copyright (c) 2013-2017, EPFL/Blue Brain Project
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

#ifndef BRAIN_TYPES
#define BRAIN_TYPES

#include <brion/types.h>

/** @namespace brain Algorithmic interface to Blue Brain data model */
namespace brain
{

/**
 * Loading of data during SynapsesStream::read(), otherwise load happens
 * on-demand.
 */
enum class SynapsePrefetch
{
    none = 0,                    //!< only loads pre- and post GIDs
    attributes = 1 << 0,         //!< topological information (section,
                                 //!  segment, distance) and model
                                 //!  attributes
    positions = 1 << 1,          //!< pre/post surface/center positions
    all = attributes | positions //!< all synapse data
};

class Circuit;
class SpikeReportReader;
class SpikeReportWriter;
class Synapse;
class Synapses;
class SynapsesIterator;
class SynapsesStream;

using vmml::Matrix4f;
using vmml::Quaternionf;
using vmml::Vector2i;
using vmml::Vector3f;
using vmml::Vector4f;

using brion::GIDSet;
using brion::Strings;
using brion::URI;
using brion::URIs;
using brion::Vector2is;
using brion::Vector3fs;
using brion::Vector4fs;
using brion::floats;
using brion::uint32_ts;
using brion::size_ts;

using brion::SectionOffsets;
using brion::Spike;
using brion::Spikes;
using brion::CompartmentCounts;

typedef std::vector< Matrix4f > Matrix4fs;
typedef std::vector< Quaternionf > Quaternionfs;

typedef std::shared_ptr< SpikeReportReader > SpikeReportReaderPtr;
typedef std::shared_ptr< SpikeReportWriter > SpikeReportWriterPtr;

/**
 * The GID of a synapse is the a tuple of two numbers:
 * - The GID of the post-synaptic cell.
 * - The index of the synapse in the array of afferent contacts
 *   of the post-synaptic cell before pruning/filtering.
 * GIDs are invariant regardless of how the structural touches are
 * converted into functional synapses during circuit building.
 */
typedef std::pair< uint32_t, size_t > SynapseGID;

namespace detail { struct SynapsesStream; }

}
#endif
