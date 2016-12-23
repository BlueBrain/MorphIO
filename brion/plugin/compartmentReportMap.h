/* Copyright (c) 2014-2017, EPFL/Blue Brain Project
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

#ifndef BRION_PLUGIN_COMPARTMENTREPORTMAP
#define BRION_PLUGIN_COMPARTMENTREPORTMAP

#include "compartmentReportCommon.h"
#include <lunchbox/stdExt.h>
#include <keyv/Map.h>

namespace brion
{
namespace plugin
{
/** A read/write report using a keyv::Map as backend */
class CompartmentReportMap : public CompartmentReportCommon
{
public:
    explicit CompartmentReportMap( const CompartmentReportInitData& initData );
    virtual ~CompartmentReportMap();

    static bool handles( const CompartmentReportInitData& initData );

    float getStartTime() const final { return _header.startTime; }
    float getEndTime() const final { return _header.endTime; }
    float getTimestep() const final { return _header.timestep; }

    const std::string& getDataUnit() const final { return _dunit; }
    const std::string& getTimeUnit() const final { return _tunit; }

    const brion::GIDSet& getGIDs() const final { return _gids; }
    const SectionOffsets& getOffsets() const final { return _offsets; }
    size_t getFrameSize() const final { return _totalCompartments; }
    const CompartmentCounts& getCompartmentCounts() const final
        { return _counts; }

    floatsPtr loadFrame( float timestamp ) const final;
    floatsPtr loadNeuron( uint32_t gid ) const final;

    void updateMapping( const GIDSet& gids ) final;

    void writeHeader( float startTime, float endTime,
                      float timestep, const std::string& dunit,
                      const std::string& tunit ) final;
    bool writeCompartments( uint32_t gid, const uint16_ts& counts ) final;
    bool writeFrame( uint32_t gid, const floats& voltages,
                     float timestamp ) final;
    bool flush() final;

    struct Header
    {
        Header();
        uint32_t magic;
        uint32_t version;
        uint32_t nGIDs; // redundant, but opt for fetching the GIDSet
        float startTime;
        float endTime;
        float timestep;
    };

private:
    const std::string _uri;
    std::vector< keyv::Map > _stores;

    Header _header;

    std::string _dunit;
    std::string _tunit;

    brion::GIDSet _gids;

    // index to get value from voltage buffer for all sections (ordered by
    // morphological description) for each cell. Cells are indexed by _gidIndex
    // (depends on the selected cells, given by updateMapping(), stored in
    // CompartmentReport::_gids
    SectionOffsets _offsets;

    // num compartments for all sections (ordered by morphological description,
    // 0s possible) for each cell. Cells are indexed by _gidIndex (depends on
    // the selected cells, given by updateMapping(), stored in
    // CompartmentReport::_gids
    brion::CompartmentCounts _counts;

    // total number of compartments (equals the size of the voltage buffer)
    uint64_t _totalCompartments;

    // <GID, num compartments per section>
    typedef std::map< uint32_t, brion::uint16_ts > CellCompartments;
    CellCompartments _cellCounts;

    bool _readable;

    void _clear();
    bool _loadHeader();
    bool _flushHeader();
};

}
}

#endif
