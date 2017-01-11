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

namespace brion
{
namespace plugin
{

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
    /** Create a new Binary report. */
    explicit SpikeReportBinary( const SpikeReportInitData& initData );

    /** Check if this plugin can handle the given uri. */
    static bool handles( const SpikeReportInitData& initData );
    static std::string getDescription();

    const URI& getURI() const final;

    float getStartTime() const final;

    float getEndTime() const final;

    const Spikes& getSpikes() const final;

    void writeSpikes( const Spikes& spikes );

    void close();

    SpikeReport::ReadMode getReadMode() const final
        { return SpikeReport::STATIC; }

private:
    const URI _uri;
    Spikes _spikes;
};

}
}

#endif
