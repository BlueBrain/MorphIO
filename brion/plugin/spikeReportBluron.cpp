/* Copyright (c) 2013-2017, EPFL/Blue Brain Project
 *                          Raphael Dumusc <raphael.dumusc@epfl.ch>
 *                          Mohamed-Ghaith Kaabi <mohamed.kaabi@epfl.ch>
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

#include "spikeReportBluron.h"
#include "../detail/skipWhiteSpace.h"
#include "../pluginInitData.h"
#include "spikeReportTypes.h"

#include <boost/filesystem.hpp>

#include <lunchbox/pluginRegisterer.h>
#include <lunchbox/stdExt.h>

#include <cstdio>
#include <fstream>

namespace brion
{
namespace plugin
{

namespace
{
lunchbox::PluginRegisterer< SpikeReportBluron > registerer;
}

using brion::Spike;

SpikeReportBluron::SpikeReportBluron( const SpikeReportInitData& initData )
    : SpikeReportASCII( initData )
{
    if ( initData.getAccessMode() == MODE_READ )
    {
        SpikeReportFile reader{ _uri.getPath(), BLURON_SPIKE_REPORT,
                                initData.getAccessMode()};
        SpikeMap spikes;
        reader.fillReportMap( spikes );

        _spikes.resize( spikes.size() );
        size_t i = 0;
        for ( auto& spike : spikes )
            _spikes[i++] = {spike.first, spike.second};
    }

    _lastReadPosition = _spikes.begin();
}

bool SpikeReportBluron::handles( const SpikeReportInitData& initData )
{
    const URI& uri = initData.getURI();
    if ( !uri.getScheme().empty() && uri.getScheme() != "file" )
        return false;

    const boost::filesystem::path ext =
        boost::filesystem::path( uri.getPath() ).extension();
    return ext == brion::plugin::BLURON_REPORT_FILE_EXT; // .dat
}

std::string SpikeReportBluron::getDescription()
{
    return "Blue Brain ASCII spike reports: "
           "[file://]/path/to/report" +
           std::string( BLURON_REPORT_FILE_EXT );
}

void SpikeReportBluron::close()
{
}

void SpikeReportBluron::write( const Spikes& spikes )
{
    if ( !spikes.size() )
        return;

    std::fstream file{getURI().getPath(),
                      std::ios_base::binary | std::ios::out | std::ios::app};
    if ( !file.is_open() )
    {
        _state = State::failed;
        return;
    }

    for ( const Spike& spike : spikes )
        file << spike.first << " " << spike.second << "\n";

    file.flush();

    _currentTime = spikes.rbegin()->first + std::numeric_limits< float >::epsilon();
}
}
}
