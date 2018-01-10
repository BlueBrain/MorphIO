/* Copyright (c) 2013-2016, EPFL/Blue Brain Project
 *                          Daniel Nachbaur <daniel.nachbaur@epfl.ch>
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

/** @file brion/enums.h Enumerations used in Brion. */

#ifndef BRION_ENUMS
#define BRION_ENUMS

#include <ostream>

namespace brion
{
namespace enums
{
/** The supported versions for morphology files. */
enum MorphologyVersion
{
    MORPHOLOGY_VERSION_H5_1 = 1,
    MORPHOLOGY_VERSION_H5_2 = 2,
    MORPHOLOGY_VERSION_H5_1_1 = 3,
    MORPHOLOGY_VERSION_SWC_1 = 101,
    MORPHOLOGY_VERSION_UNDEFINED
};

/** The cell family represented by brion::Morphology. */
enum CellFamily
{
    FAMILY_NEURON = 0,
    FAMILY_GLIA = 1
};

/** Output stream formatter for MorphologyVersion */
inline std::ostream& operator<<(std::ostream& os, const MorphologyVersion v)
{
    switch (v)
    {
    case MORPHOLOGY_VERSION_H5_1:
        return os << "h5v1";
    case MORPHOLOGY_VERSION_H5_2:
        return os << "h5v2";
    case MORPHOLOGY_VERSION_SWC_1:
        return os << "swcv1";
    default:
    case MORPHOLOGY_VERSION_UNDEFINED:
        return os << "UNDEFINED";
    }
}

/** The supported attributes of a neuron. */
enum NeuronAttributes
{
    NEURON_MORPHOLOGY_NAME = 1 << 0,
    NEURON_MORPHOLOGY_ORIGIN = 1 << 1,
    NEURON_COLUMN_GID = 1 << 2,
    NEURON_MINICOLUMN_GID = 1 << 3,
    NEURON_LAYER = 1 << 4,
    NEURON_MTYPE = 1 << 5,
    NEURON_ETYPE = 1 << 6,
    NEURON_POSITION_X = 1 << 7,
    NEURON_POSITION_Y = 1 << 8,
    NEURON_POSITION_Z = 1 << 9,
    NEURON_ROTATION = 1 << 10,
    NEURON_METYPE = 1 << 11,
    NEURON_ALL = 12,

    NEURON_ALL_ATTRIBUTES = (1 << NEURON_ALL) - 1
};

/** The supported classifications for a neuron. */
enum NeuronClass
{
    NEURONCLASS_MTYPE = 0,            //!< L2PC, L5CSPC, ...
    NEURONCLASS_MORPHOLOGY_CLASS = 1, //!< PYR, INT
    NEURONCLASS_FUNCTION_CLASS = 2,   //!< EXC, INH
    NEURONCLASS_ETYPE,                //!< cAD, dAL, ...
    NEURONCLASS_INVALID
};

/** Classification of neuron substructures. */
enum SectionType
{
    SECTION_UNDEFINED = 0,
    SECTION_SOMA = 1, //!< neuron cell body
    SECTION_AXON = 2,
    SECTION_DENDRITE = 3,        //!< general or basal dendrite (near to soma)
    SECTION_APICAL_DENDRITE = 4, //!< apical dendrite (far from soma)
    SECTION_GLIA_PROCESS = 2,
    SECTION_GLIA_ENDFOOT = 3
};

/**
 * Specify the access mode of data.
 * @version 1.4
 */
enum AccessMode
{
    MODE_READ = 0x00000001,
    MODE_WRITE = 0x00000002,
    MODE_OVERWRITE = 0x00000004 | MODE_WRITE,
    MODE_READWRITE = MODE_READ | MODE_WRITE,
    MODE_READOVERWRITE = MODE_READ | MODE_OVERWRITE
};
}
}

#endif
