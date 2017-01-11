/* Copyright (c) 2015-2017, EPFL/Blue Brain Project
 *                     Stefan.Eilemann@epfl.ch
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

#ifndef BRION_PLUGIN_COMPARTMENTREPORTNULL
#define BRION_PLUGIN_COMPARTMENTREPORTNULL

#include "compartmentReportCommon.h"

namespace brion
{
namespace plugin
{
/**
 * A read/write report using "/dev/null".
 *
 * Mostly useful in write mode for benchmarking.
 */
class CompartmentReportNull : public CompartmentReportCommon
{
public:
    explicit CompartmentReportNull( const CompartmentReportInitData& ) {}
    virtual ~CompartmentReportNull() {}

    static bool handles( const CompartmentReportInitData& initData )
        { return initData.getURI().getScheme() == "null"; }
    static std::string getDescription() { return "Benchmark drain: null://"; }

    float getStartTime() const final { return 0.f; }
    float getEndTime() const final { return 0.f; }
    float getTimestep() const final { return 0.f; }

    const std::string& getDataUnit() const final { return _emptyString; }
    const std::string& getTimeUnit() const final { return _emptyString; }
    const brion::GIDSet& getGIDs() const final { return _emptyGIDs; }
    const SectionOffsets& getOffsets() const final { return _emptyOffsets; }
    size_t getFrameSize() const final { return 0; }
    const CompartmentCounts& getCompartmentCounts() const final
        { return _emptyCounts; }

    floatsPtr loadFrame( float ) const final { return floatsPtr(); }
    void updateMapping( const GIDSet& ) final {}
    void writeHeader( float, float, float, const std::string&,
                      const std::string& ) final {}
    bool writeCompartments( uint32_t, const uint16_ts& ) final
        { return true; }
    bool writeFrame( uint32_t, const floats&, float ) final
        { return true; }
    bool flush() final { return true; }

private:
    static const std::string _emptyString;
    static const brion::GIDSet _emptyGIDs;
    static const SectionOffsets _emptyOffsets;
    static const CompartmentCounts _emptyCounts;
};

}
}

#endif
