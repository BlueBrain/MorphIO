/* Copyright (c) 2017, EPFL/Blue Brain Project
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

#ifndef BRION_PLUGIN_SPIKEREPORTASCII_H
#define BRION_PLUGIN_SPIKEREPORTASCII_H

#include "spikeReportFile.h"
#include "../pluginInitData.h"

#include <brion/types.h>
#include <brion/spikeReportPlugin.h>


namespace brion
{
namespace plugin
{

class SpikeReportASCII : public SpikeReportPlugin
{
public:
    explicit SpikeReportASCII( const SpikeReportInitData& initData );

    Spikes read(float min) final;
    Spikes readUntil(float toTimeStamp) final;
    void readSeek(float toTimeStamp) final;
    void writeSeek(float toTimeStamp) final;

protected:
    Spikes _spikes;
    Spikes::iterator _lastReadPosition;
};

}
}

#endif //BRION_PLUGIN_SPIKEREPORTFILEBASED_H
