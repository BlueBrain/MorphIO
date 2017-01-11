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

#include "spikeReportBluron.h"

#include "../pluginInitData.h"

#include <boost/lexical_cast.hpp>
#include <boost/filesystem.hpp>
#include <lunchbox/pluginRegisterer.h>
#include <lunchbox/stdExt.h>

#define BLURON_REPORT_FILE_EXT ".dat"

namespace brion
{
namespace plugin
{
namespace
{
    lunchbox::PluginRegisterer< SpikeReportBluron > registerer;
}

SpikeReportBluron::SpikeReportBluron( const SpikeReportInitData& initData )
    : _uri( initData.getURI( ))
    , _spikeReportFile( _uri.getPath(), BLURON_SPIKE_REPORT,
                        initData.getAccessMode( ))
{
    if( initData.getAccessMode() == MODE_READ )
        _spikeReportFile.fillReportMap( _spikes );
}

bool SpikeReportBluron::handles( const SpikeReportInitData& initData )
{
    const URI& uri = initData.getURI();

    if( !uri.getScheme().empty() && uri.getScheme() != "file" )
        return false;

    const boost::filesystem::path ext =
            boost::filesystem::path( uri.getPath() ).extension();
    return ext == BLURON_REPORT_FILE_EXT;
}

std::string SpikeReportBluron::getDescription()
{
    return "Blue Brain ASCII spike reports:\n"
           "  [file://]/path/to/report" BLURON_REPORT_FILE_EXT;
}

const URI& SpikeReportBluron::getURI() const
{
    return _uri;
}

float SpikeReportBluron::getStartTime() const
{
    if ( _spikes.empty( ))
        return std::numeric_limits< float >::max();

    return _spikes.begin()->first;
}

float SpikeReportBluron::getEndTime() const
{
    if ( _spikes.empty( ))
        return std::numeric_limits< float >::max();

    return _spikes.rbegin()->first;
}

const Spikes& SpikeReportBluron::getSpikes() const
{
    return _spikes;
}

void SpikeReportBluron::writeSpikes( const Spikes &spikes )
{
    _spikeReportFile.writeReportMap( spikes );
}

void SpikeReportBluron::close()
{
    _spikeReportFile.close();
}

}
}
