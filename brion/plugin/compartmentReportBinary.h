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

#ifndef BRION_PLUGIN_COMPARTMENTREPORTBINARY
#define BRION_PLUGIN_COMPARTMENTREPORTBINARY

#include "compartmentReportCommon.h"

#include <lunchbox/bitOperation.h>
#include <lunchbox/memoryMap.h>

namespace brion
{
namespace plugin
{
struct HeaderInfo
{
    int32_t headerSize;
    int32_t numCells;
    int32_t numCompartments;
    int32_t extraMappingSize;
    int32_t numFrames;
    int32_t mappingSize;
    double identifier;

    std::string dataUnit;
    std::string timeUnit;
    std::string libVersion;
    std::string simVersion;
    std::string mappingName;
    std::string extraMappingName;
    std::string reportName;

    // cppcheck-suppress unusedStructMember
    uint64_t dataBlockOffset;
    // cppcheck-suppress unusedStructMember
    bool byteswap;
};

class CompartmentReportBinary : public CompartmentReportCommon
{
public:
    explicit CompartmentReportBinary(const CompartmentReportInitData& data);
    virtual ~CompartmentReportBinary();

    static bool handles(const CompartmentReportInitData& initData);
    static std::string getDescription();

    double getStartTime() const final { return _startTime; }
    double getEndTime() const final { return _endTime; }
    double getTimestep() const final { return _timestep; }
    const std::string& getDataUnit() const final { return _dunit; }
    const std::string& getTimeUnit() const final { return _tunit; }
    const GIDSet& getGIDs() const final;
    const SectionOffsets& getOffsets() const final;
    const CompartmentCounts& getCompartmentCounts() const final;
    size_t getFrameSize() const final;

    floatsPtr loadNeuron(const uint32_t gid) const final;

    void updateMapping(const GIDSet& gids) final;

    void writeHeader(double startTime, double endTime, double timestep,
                     const std::string& dunit, const std::string& tunit) final;
    bool writeCompartments(uint32_t gid, const uint16_ts& counts) final;
    bool writeFrame(uint32_t gid, const float* values, size_t size,
                    double timestamp) final;
    bool flush() final;

private:
    bool _parseHeader();

    bool _parseMapping();

    bool _loadFrame(size_t frameNumber, float* buffer) const final;
    bool _loadFrames(size_t startFrame, size_t count,
                     float* buffer) const final;

    bool _loadFrameMemMap(size_t frameNumber, float* buffer) const;
    void _loadFramesAIO(size_t frameNumber, size_t count, float* buffer) const;

    double _startTime;
    double _endTime;
    double _timestep;
    std::string _dunit;
    std::string _tunit;

    GIDSet _gids;

    const std::string _path;
    lunchbox::MemoryMap _file;
    int _fileDescriptor;

    HeaderInfo _header;

    SectionOffsets _perSectionOffsets[2];
    CompartmentCounts _perSectionCounts[2];
    std::vector<size_t> _perCellOffsets[2];
    std::vector<uint16_t> _perCellCounts;
    std::vector<uint32_t> _subOriginalIndices;

    size_t _subNumCompartments;

    GIDSet _originalGIDs;
    bool _subtarget;

    enum class IOapi
    {
        mmap,
        posix_aio,
    } _ioAPI;
};
}
}

namespace lunchbox
{
template <>
inline void byteswap(brion::plugin::HeaderInfo& value)
{
    byteswap(value.headerSize);
    byteswap(value.numCells);
    byteswap(value.numCompartments);
    byteswap(value.extraMappingSize);
    byteswap(value.numFrames);
    byteswap(value.mappingSize);
    byteswap(value.identifier);
}
}

#endif
