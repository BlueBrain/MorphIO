/* Copyright (c) 2016, EPFL/Blue Brain Project
 *                     Stefan.Eilemann@epfl.ch
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

#ifndef BRAIN_NEURON_TYPES
#define BRAIN_NEURON_TYPES

#include <minimorph/types.h>
#include <memory>
#include <vector>

namespace morphio
{
/** High-level interface to neuron morphologies. */
class Morphology;
class Section;
class Soma;

enum class SectionType
{
    soma = minimorph::enums::SECTION_SOMA,
        axon = minimorph::enums::SECTION_AXON,
        dendrite = minimorph::enums::SECTION_DENDRITE,
        basalDendrite = minimorph::enums::SECTION_BASAL_DENDRITE,
        apicalDendrite = minimorph::enums::SECTION_APICAL_DENDRITE,
        undefined = minimorph::enums::SECTION_UNDEFINED,
        all = minimorph::enums::SECTION_ALL
        };

typedef std::shared_ptr<Morphology> MorphologyPtr;

typedef std::vector<MorphologyPtr> Morphologies;
typedef std::vector<Section> Sections;
typedef std::vector<SectionType> SectionTypes;


using vmml::Matrix4f;
using vmml::Quaternionf;
using vmml::Vector2i;
using vmml::Vector3f;
using vmml::Vector4f;

using minimorph::Strings;
using minimorph::URI;
using minimorph::Vector2is;
using minimorph::Vector3fs;
using minimorph::Vector4fs;
using minimorph::floats;
using minimorph::uint32_ts;
using minimorph::size_ts;

// Brion exceptions
using minimorph::RawDataError;
using minimorph::SomaError;
using minimorph::IDSequenceError;
using minimorph::MultipleTrees;
using minimorph::MissingParentError;

typedef std::vector<Matrix4f> Matrix4fs;
typedef std::vector<Quaternionf> Quaternionfs;


/**
 * The GID of a synapse is the a tuple of two numbers:
 * - The GID of the post-synaptic cell.
 * - The index of the synapse in the array of afferent contacts
 *   of the post-synaptic cell before pruning/filtering.
 * GIDs are invariant regardless of how the structural touches are
 * converted into functional synapses during circuit building.
 */
typedef std::pair<uint32_t, size_t> SynapseGID;


}
#endif
