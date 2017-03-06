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

#include "../compartmentReportReader.h"
#include "brion/compartmentReport.h"
#include <lunchbox/threadPool.h>

namespace brain
{
namespace detail
{
struct CompartmentReportReader
{
    CompartmentReportReader(const brion::URI& uri_)
        : uri(uri_)
    {
        const brion::CompartmentReport report{uri, brion::MODE_READ};

        metaData.startTime = report.getStartTime();
        metaData.endTime = report.getEndTime();
        metaData.timeStep = report.getTimestep();
        metaData.timeUnit = report.getTimeUnit();
        metaData.dataUnit = report.getDataUnit();
    }

    const brion::URI uri;
    CompartmentReportMetaData metaData;
    lunchbox::ThreadPool threadPool;
};

struct CompartmentReportView
{
    CompartmentReportView(
        const std::shared_ptr<CompartmentReportReader>& readerImpl_,
        const brion::GIDSet& gids)
        : report(std::make_shared<brion::CompartmentReport>(
                     readerImpl_->uri, brion::MODE_READ, gids))
        , readerImpl{readerImpl_}
    {
        _initIndices();
    }

    std::shared_ptr<brion::CompartmentReport> report;
    std::shared_ptr<CompartmentReportReader> readerImpl;
    brain::CompartmentReportMapping mapping{this};
    brain::CompartmentReportMapping::Index indices;

private:
    inline void _initIndices();
};

void CompartmentReportView::_initIndices()
{
    size_t indicesCount = 0;
    size_t index = 0;
    auto gidCount = report->getGIDs().size();

    while (gidCount--)
        indicesCount += report->getOffsets()[index++].size();

    indices.reserve(indicesCount);
    index = 0;

    for (auto gid : report->getGIDs())
    {
        const brion::uint16_ts& compartments =
            report->getCompartmentCounts()[index];
        const brion::uint64_ts& offsets = report->getOffsets()[index];
        uint16_t section = 0;
        for (auto offset : offsets)
        {
            indices.push_back({offset, gid, section, compartments[section]});
            ++section;
        }
        ++index;
    }
}

struct CompartmentReportFrame
{
    float timeStamp = 0;
    brion::floats data;
};
}
} // namespaces
