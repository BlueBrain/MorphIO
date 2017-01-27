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

#include "spikeReportNEST.h"
#include "../pluginInitData.h"
#include "spikeReportBluron.h"

#include <boost/algorithm/string.hpp>
#include <lunchbox/pluginRegisterer.h>

#include <boost/filesystem.hpp>
#include <boost/regex.hpp>

#include <fstream>

using brion::Spike;

namespace brion
{
namespace plugin
{
namespace
{
lunchbox::PluginRegisterer< SpikeReportNEST > registerer;
}

boost::regex convertToRegex( const std::string& stringWithShellLikeWildcard )
{
    std::string wildcard( stringWithShellLikeWildcard );
    boost::replace_all( wildcard, ".", "\\." );
    boost::replace_all( wildcard, "*", ".*" );
    boost::replace_all( wildcard, "/", "\\/" );
    return boost::regex( "^" + wildcard + "$" );
}

Strings expandShellWildcard( const std::string& filename )
{
    Strings expandedFilenames;

    namespace fs = boost::filesystem;

    const fs::path& filePath( filename );
    const fs::path& parent = filePath.parent_path();

    if ( !fs::exists( parent ) || !fs::is_directory( parent ) )
        LBTHROW( std::runtime_error( "Not a valid path" ) );

    // Convert the filename with shell-like wildcard into a POSIX regex
    const boost::regex regex = convertToRegex( filename );

    for ( fs::directory_iterator it( parent ); it != fs::directory_iterator(); ++it )
    {
        const std::string& candidate = it->path().string();
        if ( boost::regex_match( candidate, regex ) )
            expandedFilenames.push_back( candidate );
    }

    return expandedFilenames;
}

SpikeReportNEST::SpikeReportNEST( const SpikeReportInitData& initData )
    : SpikeReportASCII( initData )
{
    const int accessMode = initData.getAccessMode();

    if ( accessMode == MODE_READ )
    {
        const Strings& reportFiles = expandShellWildcard( _uri.getPath() );

        if ( reportFiles.empty() )
            LBTHROW(
                std::runtime_error( "No file(s) to read found in " + _uri.getPath() ) );

        for ( auto& filePath : reportFiles )
        {
            if ( !boost::filesystem::exists( filePath ) )
            {
                LBTHROW( std::runtime_error( "Cannot find file:'" + filePath + "'." ) );
            }
        }

        SpikeMap spikes;
        for ( const std::string& reportFile : reportFiles )
        {
            SpikeReportFile reader( reportFile, NEST_SPIKE_REPORT, MODE_READ );
            reader.fillReportMap( spikes );
        }
        _spikes.resize( spikes.size() );
        size_t i = 0;
        for ( auto& spike : spikes )
        {
            _spikes[i++] = {spike.first, spike.second};
        }
    }

    _lastReadPosition = _spikes.begin();
}

bool SpikeReportNEST::handles( const SpikeReportInitData& initData )
{
    const URI& uri = initData.getURI();

    if ( !uri.getScheme().empty() && uri.getScheme() != "file" )
        return false;

    const boost::filesystem::path ext =
        boost::filesystem::path( uri.getPath() ).extension();
    return ext == brion::plugin::NEST_REPORT_FILE_EXT;
}

std::string SpikeReportNEST::getDescription()
{
    return "NEST spike reports: "
           "[file://]/path/to/report" +
           std::string( NEST_REPORT_FILE_EXT );
}

void SpikeReportNEST::close()
{
}

void SpikeReportNEST::write( const Spikes& spikes )
{
    if ( spikes.empty() )
        return;

    std::fstream file{getURI().getPath(),
                      std::ios_base::binary | std::ios::out | std::ios::app};

    if ( !file.is_open() )
    {
        _state = State::failed;
        return;
    }

    for ( const Spike& spike : spikes )
    {
        file << spike.second << " " << spike.first << '\n';
    }

    file << std::flush;

    _currentTime = spikes.rbegin()->first + std::numeric_limits< float >::epsilon();
}
}
} // namespaces
