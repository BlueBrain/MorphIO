/* Copyright (c) 2013-2017, EPFL/Blue Brain Project
 *                          Raphael Dumusc <raphael.dumusc@epfl.ch>
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

#ifndef BRION_PLUGIN_SPIKEREPORTNEST_H
#define BRION_PLUGIN_SPIKEREPORTNEST_H

#include <brion/types.h>
#include <brion/spikeReportPlugin.h>
#include "spikeReportFile.h"
#include "../pluginInitData.h"

namespace brion
{
namespace plugin
{

/** A NEST spike report reader. */
class SpikeReportNEST : public SpikeReportPlugin
{
public:
    /** Create a new NEST report. */
    explicit SpikeReportNEST( const SpikeReportInitData& initData );
    virtual ~SpikeReportNEST();

    const URI& getURI() const final;

    /** Check if this plugin can handle the given uri. */
    static bool handles( const SpikeReportInitData& initData );
    static std::string getDescription();

    float getStartTime() const final;

    float getEndTime() const final;

    const Spikes& getSpikes() const final;

    void writeSpikes( const Spikes& spikes ) final;

    void close() final;

    SpikeReport::ReadMode getReadMode() const final
    {
        return SpikeReport::STATIC;
    }

private:
    const URI _uri;
    Spikes _spikes;
    SpikeReportFile _spikeReportFile;
};

}
}

#endif
