/* Copyright (c) 2015, EPFL/Blue Brain Project
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

#include "compartmentReportNull.h"
#include <lunchbox/pluginRegisterer.h>

namespace brion
{
namespace plugin
{
namespace
{
    lunchbox::PluginRegisterer< CompartmentReportNull > registerer;
}

const std::string CompartmentReportNull::_emptyString;
const brion::GIDSet CompartmentReportNull::_emptyGIDs;
const SectionOffsets CompartmentReportNull::_emptyOffsets;
const CompartmentCounts CompartmentReportNull::_emptyCounts;

}
}
