/* Copyright (c) 2015-2017, EPFL/Blue Brain Project
 *                          Stefan.Eilemann@epfl.ch
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

#include "compartmentReportCommon.h"

#include <lunchbox/debug.h> // LBUNIMPLEMENTED

namespace brion
{
namespace plugin
{
/**
 * A read/write report using dummy data.
 *
 * Mostly useful in write mode for benchmarking.
 */
class CompartmentReportDummy : public CompartmentReportCommon
{
public:
    explicit CompartmentReportDummy(const CompartmentReportInitData&);
    virtual ~CompartmentReportDummy() {}
    static bool handles(const CompartmentReportInitData& initData)
    {
        return initData.getURI().getScheme() == "dummy";
    }
    static std::string getDescription();

    float getStartTime() const final;
    float getEndTime() const final;
    float getTimestep() const final;

    const std::string& getDataUnit() const final;
    const std::string& getTimeUnit() const final;
    const brion::GIDSet& getGIDs() const final { return _gids; }
    const SectionOffsets& getOffsets() const final { return _offsets; }
    size_t getFrameSize() const final;
    const CompartmentCounts& getCompartmentCounts() const final
    {
        return _counts;
    }

    floatsPtr loadFrame(float) const final;
    void updateMapping(const GIDSet&) final;
    void writeHeader(float, float, float, const std::string&,
                     const std::string&) final
    {
    }
    bool writeCompartments(uint32_t, const uint16_ts&) final { return true; }
    bool writeFrame(uint32_t, const float*, size_t, float) final
    {
        return true;
    }
    bool flush() final { return true; }
private:
    brion::GIDSet _gids;
    SectionOffsets _offsets;
    CompartmentCounts _counts;
    size_t _defaultGIDs;
    const bool _randomValues;
};
}
}
