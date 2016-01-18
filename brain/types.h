/* Copyright (c) 2013-2015, EPFL/Blue Brain Project
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

using namespace brion::enums;

class Circuit;

using vmml::Vector2i;
using vmml::Vector3f;
using vmml::Vector4f;
using vmml::Matrix4f;

using brion::Vector2is;
using brion::Vector3fs;
using brion::Vector4fs;
typedef std::vector< Matrix4f > Matrix4fs;
using brion::uint32_ts;
using brion::floats;

using brion::SectionTypes;
using brion::GIDSet;
using brion::URI;
typedef std::vector< URI > URIs;

namespace neuron
{
    class Morphology;
    typedef boost::shared_ptr< Morphology > MorphologyPtr;
    typedef std::vector< MorphologyPtr > Morphologies;
    class Section;
    typedef std::vector< Section > Sections;
    class Soma;
}


}
#endif

