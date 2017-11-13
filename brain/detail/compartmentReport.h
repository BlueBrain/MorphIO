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

#include "../compartmentReport.h"
#include "../compartmentReportMapping.h"
#include "brion/compartmentReport.h"

#include <lunchbox/threadPool.h>
#include <lunchbox/types.h>

namespace brain
{
namespace detail
{
struct CompartmentReportReader
{
    CompartmentReportReader(const brion::URI& uri_)
        : uri(uri_)
        , report{uri}
    {
        metaData.startTime = report.getStartTime();
        metaData.endTime = report.getEndTime();
        metaData.timeStep = report.getTimestep();
        metaData.timeUnit = report.getTimeUnit();
        metaData.dataUnit = report.getDataUnit();
        metaData.frameCount = report.getFrameCount();
    }

    const brion::URI uri;
    CompartmentReportMetaData metaData;
    const brion::CompartmentReport report;

    const brion::GIDSet& getGIDs() const { return report.getGIDs(); }
    size_t getCellCount() const { return report.getCellCount(); }
};

struct CompartmentReportView
{
    CompartmentReportView(
        const std::shared_ptr<CompartmentReportReader>& readerImpl_,
        const brion::GIDSet& gids)
        : report(std::make_shared<brion::CompartmentReport>(readerImpl_->uri,
                                                            brion::MODE_READ,
                                                            gids))
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
    indices.resize(report->getFrameSize());

    const auto& gids = report->getGIDs();
    const std::vector<uint32_t> gidList(report->getGIDs().begin(),
                                        report->getGIDs().end());
    size_t i = 0;
    for (auto gid : gids)
    {
        const brion::uint16_ts& compartments =
            report->getCompartmentCounts()[i];
        const brion::uint64_ts& offsets = report->getOffsets()[i];
        for (size_t section = 0; section != compartments.size(); ++section)
        {
            const auto offset = offsets[section];
            if (offset == LB_UNDEFINED_UINT64)
                continue;

            const auto count = compartments[section];
            for (size_t k = 0; k != count; ++k)
            {
                indices[offset + k].gid = gid;
                indices[offset + k].section = section;
            }
        }
        ++i;
    }
}
}
} // namespaces
