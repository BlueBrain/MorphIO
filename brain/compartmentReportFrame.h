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
#pragma once

#include <brain/api.h>
#include <brain/types.h>

namespace brain
{
namespace detail
{
struct CompartmentReportFrame;
}

/**
 * A simulation data frame.
 *
 * A frame contains the values of a scalar variable on compartments at a given
 * timestamp.
 */
class CompartmentReportFrame
{
public:
    BRAIN_API CompartmentReportFrame();
    BRAIN_API ~CompartmentReportFrame();

    BRAIN_API CompartmentReportFrame(CompartmentReportFrame&&) noexcept;
    BRAIN_API CompartmentReportFrame& operator=(
        CompartmentReportFrame&&) noexcept;

    /** @return the timestamp of the frame.
     * @version 2.0
     */
    BRAIN_API double getTimestamp() const;

    /** @return true if the frame is empty
     * @version 2.0
     */
    BRAIN_API bool empty() const;

    /** @return the data of the frame.
     * The data layout is specified by a CompartmentReportMapping.
     */
    BRAIN_API const floats& getData() const;

    BRAIN_API floats takeData();

private:
    std::unique_ptr<detail::CompartmentReportFrame> _impl;
    friend class CompartmentReportView;
};
}
