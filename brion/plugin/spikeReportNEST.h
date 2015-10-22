/* Copyright (c) 2013-2015, EPFL/Blue Brain Project
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

    /** @copydoc brion::SpikeReport::getURI */
    const URI& getURI() const final;

    /** Check if this plugin can handle the given uri. */
    static bool handles( const SpikeReportInitData& initData );

    /** @copydoc brion::SpikeReport::getStartTime */
    float getStartTime() const final;

    /** @copydoc brion::SpikeReport::getEndTime */
    float getEndTime() const final;

    /** @copydoc brion::SpikeReport::getSpikes */
    const Spikes& getSpikes() const final;

    /** @copydoc brion::SpikeReport::writeSpikes */
    void writeSpikes( const Spikes& spikes ) final;

    /** @copydoc brion::SpikeReport::close */
    void close() final;

    /** @copydoc brion::SpikeReport::getReadMode */
    SpikeReport::ReadMode getReadMode() const final
    {
        return SpikeReport::STATIC;
    }

private:
    const URI _uri;
    Spikes _spikes;
    Strings _reportFiles;
    typedef std::vector< SpikeReportFile* > SpikeReportFiles;
    SpikeReportFiles _spikeReportFiles;
};

}
}

#endif
