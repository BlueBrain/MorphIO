/* Copyright (c) 2017, EPFL/Blue Brain Project
 *                          Stefan.Eilemann@epfl.ch
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

namespace
{
// v1 & v2
const std::string _d_points("points");
const size_t _pointColumns = 4;

// v1
const std::string _d_structure("/structure");
const size_t _structureV1Columns = 3;

// v1.1
const std::string _g_metadata("/metadata");
const std::string _e_family("cell_family_enum");
const std::string _a_family("cell_family");
const std::string _d_perimeters("/perimeters");
const std::string _a_software_version("software_version");
const std::string _a_creation_time("creation_time");

// v1.1 & v2
const std::string _a_creator("creator");
const std::string _a_version("version");

// v2
const std::string _g_structure("structure");
const size_t _structureV2Columns = 2;
const std::string _g_root("neuron1");
const std::string _d_type("sectiontype");
const std::string _a_apical("apical");
}
