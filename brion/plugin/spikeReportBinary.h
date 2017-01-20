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

#ifndef BRION_PLUGIN_SPIKEREPORTBINARY_H
#define BRION_PLUGIN_SPIKEREPORTBINARY_H

#include <brion/types.h>
#include <brion/spikeReportPlugin.h>

namespace lunchbox
{
class MemoryMap;
}

namespace brion
{
namespace plugin
{

class BinaryReportFileMemMap
{
public:
    BinaryReportFileMemMap() = default;

    void mapForRead( const std::string& path );
    void mapForReadWrite( const std::string& path, size_t spikeCount );

    size_t getSpikeCount() const;
    Spike* getSpikes() const;

    bool isMappedForRead()const;

    operator bool() const;

private:
    std::unique_ptr< lunchbox::MemoryMap > _memFile;
    Spike* _begin = nullptr;
    size_t _spikeCount = 0;
    bool _mappedForRead = false;
};

/**
 * A Binary spike report reader.
 *
 * The format read by this plugin is:
 * - 4b integer: magic '0xf0a'
 * - 4b integer: version, currently '1'
 * - (4b float, 4b integer) pairs until end of file: spike time, neuron GID,
 *   sorted by time
 */
class SpikeReportBinary : public SpikeReportPlugin
{
public:
    explicit SpikeReportBinary( const SpikeReportInitData& initData );

    static bool handles( const SpikeReportInitData& initData );
    static std::string getDescription();

    void close() final;
    Spikes read( float min ) final;
    Spikes readUntil( float max ) final;
    void readSeek( float toTimeStamp ) final;
    void writeSeek( float toTimeStamp ) final;
    void write( const Spikes& spikes ) final;

private:
    BinaryReportFileMemMap _memFile;
    size_t _startIndex = 0;
};
}
}

#endif
