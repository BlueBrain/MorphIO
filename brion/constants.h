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

#ifndef BRION_CONSTANTS
#define BRION_CONSTANTS

namespace brion
{
const char* const SPIKE_FILE = "/out.dat";
const char* const CIRCUIT_FILE_MVD2 = "/circuit.mvd2";
const char* const CIRCUIT_FILE_MVD3 = "/circuit.mvd3";
const char* const CIRCUIT_TARGET_FILE = "/start.target";

const char* const MORPHOLOGY_HDF5_FILES_SUBDIRECTORY = "h5";

const char* const BLUECONFIG_CIRCUIT_PATH_KEY = "CircuitPath";
const char* const BLUECONFIG_NRN_PATH_KEY = "nrnPath";
const char* const BLUECONFIG_MORPHOLOGY_PATH_KEY = "MorphologyPath";
const char* const BLUECONFIG_OUTPUT_PATH_KEY = "OutputRoot";
const char* const BLUECONFIG_TARGET_FILE_KEY = "TargetFile";
const char* const BLUECONFIG_SPIKES_PATH_KEY = "SpikesPath";

const char* const BLUECONFIG_CIRCUIT_TARGET_KEY = "CircuitTarget";
const char* const BLUECONFIG_REPORT_FORMAT_KEY = "Format";
const char* const BLUECONFIG_DT_KEY = "Dt";

const char* const BLUECONFIG_PROJECTION_PATH_KEY = "Path";
}

#endif
