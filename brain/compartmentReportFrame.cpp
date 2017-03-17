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
#include "compartmentReportFrame.h"
#include "detail/compartmentReport.h"

namespace brain
{
CompartmentReportFrame::CompartmentReportFrame()
    : _impl{new detail::CompartmentReportFrame}
{
}

CompartmentReportFrame::~CompartmentReportFrame()
{
}

CompartmentReportFrame::CompartmentReportFrame(
    CompartmentReportFrame&& other) noexcept : _impl(std::move(other._impl))
{
}

CompartmentReportFrame& CompartmentReportFrame::operator=(
    CompartmentReportFrame&& other) noexcept
{
    if (&other == this)
        return *this;
    _impl = std::move(other._impl);
    return *this;
}

bool CompartmentReportFrame::empty() const
{
    return _impl->data.empty();
}

const brion::floats& CompartmentReportFrame::getData() const
{
    return _impl->data;
}

brion::floats CompartmentReportFrame::takeData()
{
    return std::move(_impl->data);
}

double CompartmentReportFrame::getTimestamp() const
{
    return _impl->timeStamp;
}
}
