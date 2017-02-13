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

#include "compartmentReportReader.h"
#include "detail/compartmentReportReader.h"

namespace brain
{
CompartmentReportReader::CompartmentReportReader(const brion::URI& uri)
    : _impl{new detail::CompartmentReportReader(uri)}
{
}

CompartmentReportReader::~CompartmentReportReader()
{
}

const CompartmentReportMetaData& CompartmentReportReader::getMetaData() const
{
    return _impl->metaData;
}

CompartmentReportView CompartmentReportReader::createView(
    const brion::GIDSet& cells)
{
    return CompartmentReportView(_impl, cells);
}

CompartmentReportView CompartmentReportReader::createView()
{
    return CompartmentReportView(_impl, {});
}

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

float CompartmentReportFrame::getTimestamp() const
{
    return _impl->timeStamp;
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

std::future<CompartmentReportFrame> CompartmentReportView::load(
    const float timestamp)
{
    auto report = _impl->report;
    auto loadFrameTask = [timestamp, report] {

        CompartmentReportFrame frame;
        frame._impl->timeStamp = timestamp;

        auto data = report->loadFrame(timestamp);
        if (data)
            frame._impl->data = std::move(*data);

        return frame;
    };

    return _impl->readerImpl->threadPool.post(loadFrameTask);
}

std::future<CompartmentReportFrames> CompartmentReportView::load(float start,
                                                                 float end)
{
    if (end <= start)
        throw std::logic_error("Invalid interval");

    start = std::max(start, _impl->report->getStartTime());
    end = std::min(end, _impl->report->getEndTime());

    const float timestep = _impl->report->getTimestep();
    auto report = _impl->report;

    auto loadFrameTask = [start, end, timestep, report] {

        // If the input time window is outside the report the clamping done
        // above will make end <= start
        CompartmentReportFrames frames;
        if (end <= start)
            return frames;

        // The first timestamp is snapped to the closest smaller frame start
        // according to the timestep.
        for (size_t i = std::floor(start / timestep); i * timestep < end; ++i)
        {
            CompartmentReportFrame frame;
            const float t = i * timestep;
            frame._impl->timeStamp = t;
            auto data = report->loadFrame(t);
            if (data)
                frame._impl->data = std::move(*data);

            frames.push_back(std::move(frame));
        }
        return frames;
    };
    return _impl->readerImpl->threadPool.post(loadFrameTask);
}

std::future<CompartmentReportFrames> CompartmentReportView::loadAll()
{
    return load(_impl->report->getStartTime(), _impl->report->getEndTime());
}

CompartmentReportMapping::CompartmentReportMapping(
    detail::CompartmentReportView* view)
    : _viewImpl{view}
{
}

using Indices = CompartmentReportMapping::Index;

const Indices& CompartmentReportMapping::getIndex() const
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

} // namespace
