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

#include "spikeReportASCII.h"

#include <brion/spikeReportPlugin.h>
#include <brion/types.h>

namespace brion
{
namespace plugin
{
/** A NEST spike report reader. */
class SpikeReportNEST : public SpikeReportASCII
{
public:
    explicit SpikeReportNEST(const SpikeReportInitData& initData);

    static bool handles(const SpikeReportInitData& initData);
    static std::string getDescription();

    void close() final;
    void write(const Spike* spikes, size_t size) final;
};
}
}
#endif
