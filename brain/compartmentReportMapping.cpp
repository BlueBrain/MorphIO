/* Copyright (c) 2017, EPFL/Blue Brain Project
 *                     Juan Hernando <juan.hernando@epfl.ch>
 *                     Mohamed-Ghaith Kaabi <mohamed.kaabi@epfl.ch>
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
#include "compartmentReportMapping.h"
#include "detail/compartmentReport.h"

namespace brain
{
CompartmentReportMapping::CompartmentReportMapping(
    detail::CompartmentReportView* view)
    : _viewImpl{view}
{
}

const CompartmentReportMapping::Index& CompartmentReportMapping::getIndex()
    const
{
    return _viewImpl->indices;
}

const brion::SectionOffsets& CompartmentReportMapping::getOffsets() const
{
    return _viewImpl->report->getOffsets();
}

const brion::CompartmentCounts& CompartmentReportMapping::getCompartmentCounts()
    const
{
    return _viewImpl->report->getCompartmentCounts();
}

size_t CompartmentReportMapping::getNumCompartments(size_t index) const
{
    return _viewImpl->report->getNumCompartments(index);
}

size_t CompartmentReportMapping::getFrameSize() const
{
    return _viewImpl->report->getFrameSize();
}
}
