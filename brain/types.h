/* Copyright (c) 2013-2016, EPFL/Blue Brain Project
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
class Spikes;
class SpikeReportReader;
class SpikeReportWriter;

using vmml::Matrix4f;
using vmml::Quaternionf;
using vmml::Vector2i;
using vmml::Vector3f;
using vmml::Vector4f;

using brion::GIDSet;
using brion::SectionTypes;
using brion::Strings;
using brion::URI;
using brion::Vector2is;
using brion::Vector3fs;
using brion::Vector4fs;
using brion::floats;
using brion::uint32_ts;

typedef std::vector< Matrix4f > Matrix4fs;
typedef std::vector< Quaternionf > Quaternionfs;
typedef std::vector< URI > URIs;

typedef boost::shared_ptr< SpikeReportReader > SpikeReportReaderPtr;
typedef boost::shared_ptr< SpikeReportWriter > SpikeReportWriterPtr;
}
#endif
