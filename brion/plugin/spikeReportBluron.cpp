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
const char* const BLURON_REPORT_FILE_EXT = ".dat";
}

SpikeReportBluron::SpikeReportBluron( const SpikeReportInitData& initData )
    : SpikeReportASCII( initData )
{
    if ( initData.getAccessMode() == MODE_READ )
    {
        _spikes = parse( _uri.getPath(),
                         []( const std::string& buffer, Spike& spike )
                         {
                             return sscanf( buffer.data(), "%20f%20ud",
                                            &spike.first, &spike.second ) == 2;
                         });
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
    SpikeReportASCII::write(
        spikes, []( std::ostream& file, const Spike& spike ){
                    file << spike.first << " " << spike.second << "\n"; });
}
}
}
