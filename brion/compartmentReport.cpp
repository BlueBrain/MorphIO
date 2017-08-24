/* Copyright (c) 2013-2017, EPFL/Blue Brain Project
 *                          Daniel Nachbaur <daniel.nachbaur@epfl.ch>
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

#include "compartmentReport.h"
#include "compartmentReportPlugin.h"

#include <lunchbox/log.h>
#include <lunchbox/pluginFactory.h>
#include <lunchbox/threadPool.h>

namespace brion
{
namespace
{
using CompartmentPluginFactory =
    lunchbox::PluginFactory<CompartmentReportPlugin>;
}

namespace detail
{
class CompartmentReport
{
public:
    explicit CompartmentReport(const CompartmentReportInitData& initData)
        : plugin(CompartmentPluginFactory::getInstance().create(initData))
    {
    }

    const std::unique_ptr<CompartmentReportPlugin> plugin;
};
}

CompartmentReport::CompartmentReport(const URI& uri, const int mode,
                                     const GIDSet& gids)
    : _impl(new detail::CompartmentReport(
          CompartmentReportInitData(uri, mode, gids)))
{
}

CompartmentReport::~CompartmentReport()
{
    delete _impl;
}

std::string CompartmentReport::getDescriptions()
{
    return CompartmentPluginFactory::getInstance().getDescriptions();
}

const GIDSet& CompartmentReport::getGIDs() const
{
    return _impl->plugin->getGIDs();
}

size_t CompartmentReport::getIndex(const uint32_t gid) const
{
    return _impl->plugin->getIndex(gid);
}

const SectionOffsets& CompartmentReport::getOffsets() const
{
    return _impl->plugin->getOffsets();
}

const CompartmentCounts& CompartmentReport::getCompartmentCounts() const
{
    return _impl->plugin->getCompartmentCounts();
}

size_t CompartmentReport::getNumCompartments(const size_t index) const
{
    return _impl->plugin->getNumCompartments(index);
}

double CompartmentReport::getStartTime() const
{
    return _impl->plugin->getStartTime();
}

double CompartmentReport::getEndTime() const
{
    return _impl->plugin->getEndTime();
}

double CompartmentReport::getTimestep() const
{
    return _impl->plugin->getTimestep();
}

const std::string& CompartmentReport::getDataUnit() const
{
    return _impl->plugin->getDataUnit();
}

const std::string& CompartmentReport::getTimeUnit() const
{
    return _impl->plugin->getTimeUnit();
}

size_t CompartmentReport::getFrameSize() const
{
    return _impl->plugin->getFrameSize();
}

size_t CompartmentReport::getFrameCount() const
{
    return _impl->plugin->getFrameCount();
}

size_t CompartmentReport::getBufferSize() const
{
    return _impl->plugin->getBufferSize();
}

std::future<floatsPtr> CompartmentReport::loadFrame(
    const double timestamp) const
{
    auto task = [timestamp, this] {
        if (timestamp < getStartTime() || timestamp >= getEndTime())
            return floatsPtr();

        return _impl->plugin->loadFrame(timestamp);
    };
    return lunchbox::ThreadPool::getInstance().post(task);
}

std::future<Frames> CompartmentReport::loadFrames(const double start,
                                                  const double end) const
{
    auto task = [start, end, this] {
        if (end < getStartTime() || start >= getEndTime())
            return Frames();
        return _impl->plugin->loadFrames(start, end);
    };
    return lunchbox::ThreadPool::getInstance().post(task);
}

size_t CompartmentReport::getNeuronSize(const uint32_t gid) const
{
    const size_t index = getIndex(gid);
    const double step = getTimestep();
    const size_t nTimesteps =
        // Added timestep/2 to avoid round-off errors.
        (getEndTime() - getStartTime() + step * 0.5) / step;
    return getNumCompartments(index) * nTimesteps;
}

std::future<floatsPtr> CompartmentReport::loadNeuron(const uint32_t gid) const
{
    auto task = [gid, this] { return _impl->plugin->loadNeuron(gid); };
    return lunchbox::ThreadPool::getInstance().post(task);
}

void CompartmentReport::setBufferSize(const size_t size)
{
    _impl->plugin->setBufferSize(size);
}

void CompartmentReport::clearBuffer()
{
    _impl->plugin->clearBuffer();
}

void CompartmentReport::updateMapping(const GIDSet& gids)
{
    _impl->plugin->updateMapping(gids);
}

void CompartmentReport::writeHeader(const double startTime,
                                    const double endTime, const double timestep,
                                    const std::string& dunit,
                                    const std::string& tunit)
{
    _impl->plugin->writeHeader(startTime, endTime, timestep, dunit, tunit);
}

bool CompartmentReport::writeCompartments(const uint32_t gid,
                                          const uint16_ts& counts)
{
    return _impl->plugin->writeCompartments(gid, counts);
}

bool CompartmentReport::writeFrame(uint32_t gid, const float* values,
                                   const size_t size, double timestamp)
{
    return _impl->plugin->writeFrame(gid, values, size, timestamp);
}

bool CompartmentReport::flush()
{
    return _impl->plugin->flush();
}

bool CompartmentReport::erase()
{
    return _impl->plugin->erase();
}
}
