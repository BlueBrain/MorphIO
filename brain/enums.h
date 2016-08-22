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

/** @file brain/enums.h Enumerations used in Brion. */

#ifndef BRAIN_ENUMS
#define BRAIN_ENUMS

#include <brion/types.h>

namespace brain
{
namespace enums
{

using namespace brion::enums;

/**
 * Loading of data during SynapsesStream::read(), otherwise load happens
 * on-demand.
 */
enum SynapsePrefetch
{
    SYNAPSEPREFETCH_NONE = 0,            //!< only loads pre- and post GIDs
    SYNAPSEPREFETCH_ATTRIBUTES = 1 << 0, //!< topological information (section,
                                         //!  segment, distance) and model
                                         //!  attributes
    SYNAPSEPREFETCH_POSITIONS = 1 << 1,  //!< pre/post surface/center positions
    SYNAPSEPREFETCH_ALL =                //!< all synapse data
        SYNAPSEPREFETCH_ATTRIBUTES | SYNAPSEPREFETCH_POSITIONS
};

}
}
#endif
