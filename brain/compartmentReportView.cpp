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
namespace
{
bool _isMultiple(const double a, const double b)
{
    const auto remainder = std::fmod(a, b);
    // Using double epsilon is too strict for large values of a/b
    const auto epsilon = b * std::numeric_limits<float>::epsilon();
    return remainder <= epsilon || (b - remainder) <= epsilon;
}
}

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
inline double _snapTimestamp(double t, double start, double timestep)
{
    return start + timestep * (size_t)std::floor((t - start) / timestep);
}
}

std::future<brion::Frame> CompartmentReportView::load(double timestamp)
{
    const double start = _impl->report->getStartTime();
    const double end = _impl->report->getEndTime();

    if (timestamp < start || timestamp >= end)
        throw std::logic_error("Invalid timestamp");

    const double timestep = _impl->report->getTimestep();

    timestamp = _snapTimestamp(timestamp, start, timestep);

    auto report = _impl->report;

    auto loadFrameTask = [timestamp, report] {

        auto data = report->loadFrame(timestamp).get();
        if (data)
            return brion::Frame{timestamp, data};
        return brion::Frame{0, brion::floatsPtr()};
    };

    return lunchbox::ThreadPool::getInstance().post(loadFrameTask);
}

std::future<brion::Frames> CompartmentReportView::load(double start, double end)
{
    if (end <= start)
        throw std::logic_error("Invalid interval");

    start = std::max(start, (double)_impl->report->getStartTime());
    end = std::min(end, (double)_impl->report->getEndTime());

    return _impl->report->loadFrames(start, end);
}

std::future<brion::Frames> CompartmentReportView::load(double start, double end,
                                                       const double step)
{
    const double reportTimeStep = _impl->report->getTimestep();
    const double reportStartTime = _impl->report->getStartTime();

    if (end <= start)
        throw std::logic_error("Invalid interval");

    if (step < reportTimeStep || step <= 0.)
        throw std::logic_error("Invalid step");
    if (!_isMultiple(step, reportTimeStep))
        throw std::logic_error(
            "Step should be a multiple of the report time step");

    // Making sure the timestamps we are going to request always fall in the
    // middle of a frame. For that we snap start to the beginning of the frame
    // it's contained and then we add half the time step.
    start = std::max(start, _impl->report->getStartTime());
    size_t frameIndex = (start - reportStartTime) / reportTimeStep;
    start = (frameIndex + 0.5) * reportTimeStep + reportStartTime;

    end = std::min(end, _impl->report->getEndTime());

    auto task = [this, start, end, step] {

        brion::Frames frames;
        frames.timeStamps.reset(new brion::doubles);
        frames.data.reset(new floats);

        double t = start;
        uint32_t i = 0;
        while (t < end)
        {
            auto frame = load(t).get();
            frames.timeStamps->push_back(frame.timestamp);
            std::copy(frame.data->begin(), frame.data->end(),
                      std::back_inserter(*frames.data));
            ++i;
            t = start + i * step;
        }

        return frames;
    };

    return lunchbox::ThreadPool::getInstance().post(task);
}

std::future<brion::Frames> CompartmentReportView::loadAll()
{
    return load(_impl->report->getStartTime(), _impl->report->getEndTime());
}
}
