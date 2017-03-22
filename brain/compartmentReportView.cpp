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
#include "compartmentReportView.h"
#include "detail/compartmentReport.h"

namespace brain
{
CompartmentReportView::CompartmentReportView(
    const std::shared_ptr<detail::CompartmentReportReader>& readerImpl,
    const brion::GIDSet& gids)
    : _impl(new detail::CompartmentReportView(readerImpl, gids))
{
}

CompartmentReportView::CompartmentReportView(CompartmentReportView&& other)
{
    _impl = std::move(other._impl);
}

CompartmentReportView& CompartmentReportView::operator=(
    CompartmentReportView&& other)
{
    _impl = std::move(other._impl);
    return *this;
}

CompartmentReportView::~CompartmentReportView()
{
}

const brion::GIDSet& CompartmentReportView::getGIDs() const
{
    return _impl->report->getGIDs();
}

const CompartmentReportMapping& CompartmentReportView::getMapping() const
{
    return _impl->mapping;
}

namespace
{
// using float is necessary to match brion::CompartmentReport
// frame index calculation
inline float _snapTimestamp(float t, float start, float timestep)
{
    return start + timestep * (size_t)std::floor((t - start) / timestep);
}
}

std::future<CompartmentReportFrame> CompartmentReportView::load(
    double timestamp)
{
    const double start = _impl->report->getStartTime();
    const double end = _impl->report->getEndTime();

    if (timestamp < start || timestamp >= end)
        throw std::logic_error("Invalid timestamp");

    const double timestep = _impl->report->getTimestep();

    timestamp = _snapTimestamp(timestamp, start, timestep);

    auto report = _impl->report;

    auto loadFrameTask = [timestamp, report] {

        CompartmentReportFrame frame;
        frame._impl->timeStamp = timestamp;

        auto data = report->loadFrame(timestamp).get();
        if (data)
            frame._impl->data = std::move(*data);

        return frame;
    };

    return _impl->readerImpl->threadPool.post(loadFrameTask);
}

std::future<brion::Frames> CompartmentReportView::load(double start, double end)
{
    if (end <= start)
        throw std::logic_error("Invalid interval");

    start = std::max(start, (double)_impl->report->getStartTime());
    end = std::min(end, (double)_impl->report->getEndTime());

    return _impl->report->loadFrames(start, end);
}

std::future<brion::Frames> CompartmentReportView::loadAll()
{
    return load(_impl->report->getStartTime(), _impl->report->getEndTime());
}
}
