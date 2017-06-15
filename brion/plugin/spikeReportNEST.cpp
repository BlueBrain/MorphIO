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

#include <lunchbox/pluginRegisterer.h>

#include <boost/algorithm/string.hpp>
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
lunchbox::PluginRegisterer<SpikeReportNEST> registerer;
const char* const NEST_REPORT_FILE_EXT = ".gdf";
}

boost::regex convertToRegex(const std::string& stringWithShellLikeWildcard)
{
    std::string wildcard(stringWithShellLikeWildcard);
    boost::replace_all(wildcard, ".", "\\.");
    boost::replace_all(wildcard, "*", ".*");
    boost::replace_all(wildcard, "/", "\\/");
    return boost::regex("^" + wildcard + "$");
}

Strings expandShellWildcard(const std::string& filename)
{
    Strings expandedFilenames;

    namespace fs = boost::filesystem;

    const fs::path& filePath(filename);
    const fs::path& parent = filePath.parent_path();

    if (!fs::exists(parent) || !fs::is_directory(parent))
        LBTHROW(std::runtime_error("Not a valid path"));

    // Convert the filename with shell-like wildcard into a POSIX regex
    const boost::regex regex = convertToRegex(filename);

    for (fs::directory_iterator it(parent); it != fs::directory_iterator();
         ++it)
    {
        const std::string& candidate = it->path().string();
        if (boost::regex_match(candidate, regex))
            expandedFilenames.push_back(candidate);
    }

    return expandedFilenames;
}

SpikeReportNEST::SpikeReportNEST(const SpikeReportInitData& initData)
    : SpikeReportASCII(initData)
{
    const int accessMode = initData.getAccessMode();

    if (accessMode == MODE_READ)
    {
        const Strings& files = expandShellWildcard(_uri.getPath());

        if (files.empty())
            LBTHROW(std::runtime_error("No files to read found in " +
                                       _uri.getPath()));

        _spikes = parse(files, [](const std::string& buffer, Spike& spike) {
            return sscanf(buffer.data(), "%20d%20f", &spike.second,
                          &spike.first) == 2;
        });
    }

    _lastReadPosition = _spikes.begin();
    if (!_spikes.empty())
        _endTime = _spikes.rbegin()->first;
}

bool SpikeReportNEST::handles(const SpikeReportInitData& initData)
{
    const URI& uri = initData.getURI();

    if (!uri.getScheme().empty() && uri.getScheme() != "file")
        return false;

    const boost::filesystem::path ext =
        boost::filesystem::path(uri.getPath()).extension();
    return ext == brion::plugin::NEST_REPORT_FILE_EXT;
}

std::string SpikeReportNEST::getDescription()
{
    return "NEST spike reports: "
           "[file://]/path/to/report" +
           std::string(NEST_REPORT_FILE_EXT);
}

void SpikeReportNEST::close()
{
}

void SpikeReportNEST::write(const Spike* spikes, const size_t size)
{
    append(spikes, size, [](std::ostream& file, const Spike& spike) {
        file << spike.second << " " << spike.first << '\n';
    });
}
}
} // namespaces
