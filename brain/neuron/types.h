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

#include <boost/shared_ptr.hpp>
#include <brion/enums.h>
#include <vector>

namespace brain
{
/** High-level interface to neuron morphologies. */
namespace neuron
{
class Morphology;
class Section;
class Soma;

enum class SectionType
{
    soma = brion::enums::SECTION_SOMA,
    axon = brion::enums::SECTION_AXON,
    dendrite = brion::enums::SECTION_DENDRITE,
    apicalDendrite = brion::enums::SECTION_APICAL_DENDRITE,
    undefined = brion::enums::SECTION_UNDEFINED
};

typedef boost::shared_ptr<Morphology> MorphologyPtr;

typedef std::vector<MorphologyPtr> Morphologies;
typedef std::vector<Section> Sections;
typedef std::vector<SectionType> SectionTypes;
}
}
#endif
