/* Copyright (c) 2013-2017, EPFL/Blue Brain Project
 *                          Daniel Nachbaur <daniel.nachbaur@epfl.ch>
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

#include "blueConfig.h"

#include "constants.h"
#include "target.h"

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/regex.hpp>
#include <fstream>
#include <lunchbox/log.h>
#include <unordered_map>

namespace fs = boost::filesystem;
namespace boost
{
template <>
inline brion::BlueConfigSection lexical_cast(const std::string& s)
{
    if (s == "Run")
        return brion::CONFIGSECTION_RUN;
    if (s == "Connection")
        return brion::CONFIGSECTION_CONNECTION;
    if (s == "Stimulus")
        return brion::CONFIGSECTION_STIMULUS;
    if (s == "StimulusInject")
        return brion::CONFIGSECTION_STIMULUSINJECT;
    if (s == "Report")
        return brion::CONFIGSECTION_REPORT;
    return brion::CONFIGSECTION_UNKNOWN;
}

template <>
inline std::string lexical_cast(const brion::BlueConfigSection& b)
{
    switch (b)
    {
    case brion::CONFIGSECTION_RUN:
        return "Run";
    case brion::CONFIGSECTION_CONNECTION:
        return "Connection";
    case brion::CONFIGSECTION_STIMULUS:
        return "Stimulus";
    case brion::CONFIGSECTION_STIMULUSINJECT:
        return "StimulusInject";
    case brion::CONFIGSECTION_REPORT:
        return "Report";
    default:
        return "UNKNOWN";
    }
    throw boost::bad_lexical_cast();
}
}

namespace brion
{
typedef std::unordered_map<std::string, std::string> KVStore;
typedef std::unordered_map<std::string, KVStore> ValueTable;

namespace detail
{
class BlueConfig
{
public:
    explicit BlueConfig(const std::string& source)
    {
        std::ifstream file(source.c_str());
        if (!file.is_open())
            LBTHROW(
                std::runtime_error("Cannot open BlueConfig file " + source));
        std::stringstream buffer;
        buffer << file.rdbuf();

        boost::regex commentregx("#.*?\\n");
        const std::string fileString =
            boost::regex_replace(buffer.str(), commentregx, "\n");

        boost::regex regx(
            "(?<type>[a-zA-Z0-9_-]+) (?<name>[a-zA-Z0-9_-]+?)"
            "\\s+\\{(?<contents>.*?)\\}");
        const int subs[] = {1, 2, 3};
        boost::sregex_token_iterator i(fileString.begin(), fileString.end(),
                                       regx, subs);
        for (boost::sregex_token_iterator end; i != end;)
        {
            const std::string& typeStr = *i++;
            const std::string& name = *i++;
            const std::string& content = *i++;
            if (content.empty())
            {
                LBWARN << "Found empty section '" << typeStr << " " << name
                       << "' in BlueConfig file " << source << std::endl;
                continue;
            }

            const BlueConfigSection type =
                boost::lexical_cast<BlueConfigSection>(typeStr);
            if (type == brion::CONFIGSECTION_UNKNOWN)
            {
                LBDEBUG << "Found unknown section '" << typeStr
                        << "' in BlueConfig file " << source << std::endl;
                continue;
            }

            names[type].push_back(name);

            Strings lines;
            boost::split(lines, content, boost::is_any_of("\n"),
                         boost::token_compress_on);

            BOOST_FOREACH (std::string line, lines)
            {
                boost::trim(line);
                if (line.empty())
                    continue;

                const std::string::size_type pos = line.find(' ', 0);
                if (pos == std::string::npos)
                {
                    LBWARN << "Found invalid key-value pair '" << line
                           << "' in BlueConfig file " << source << std::endl;
                    continue;
                }

                std::string value = line.substr(pos + 1);
                boost::trim(value);
                table[type][name].insert(
                    std::make_pair(line.substr(0, pos), value));
            }
        }

        if (table[CONFIGSECTION_RUN].empty())
            LBTHROW(
                std::runtime_error(source + " not a valid BlueConfig file"));
    }

    std::string getRun()
    {
        const brion::Strings& runs = names[brion::CONFIGSECTION_RUN];
        return runs.empty() ? std::string() : runs.front();
    }

    const std::string& get(const BlueConfigSection section,
                           const std::string& sectionName,
                           const std::string& key) const
    {
        // This function doesn't create entries in the tables in case they
        // don't exist.
        static std::string empty;
        const ValueTable::const_iterator tableIt =
            table[section].find(sectionName);
        if (tableIt == table[section].end())
            return empty;
        const KVStore& store = tableIt->second;
        const KVStore::const_iterator kv = store.find(key);
        if (kv == store.end())
            return empty;
        return kv->second;
    }

    const std::string& getCircuitTarget()
    {
        return get(brion::CONFIGSECTION_RUN, getRun(),
                   BLUECONFIG_CIRCUIT_TARGET_KEY);
    }

    const std::string& getOutputRoot()
    {
        return get(brion::CONFIGSECTION_RUN, getRun(),
                   BLUECONFIG_OUTPUT_PATH_KEY);
    }

    template <typename T>
    bool get(const BlueConfigSection section, const std::string& sectionName,
             const std::string& key, T& value) const
    {
        try
        {
            value = boost::lexical_cast<T>(get(section, sectionName, key));
        }
        catch (const boost::bad_lexical_cast&)
        {
            return false;
        }
        return true;
    }

    Strings names[CONFIGSECTION_ALL];
    ValueTable table[CONFIGSECTION_ALL];
};
}

BlueConfig::BlueConfig(const std::string& source)
    : _impl(new detail::BlueConfig(source))
{
}

BlueConfig::~BlueConfig()
{
    delete _impl;
}

const Strings& BlueConfig::getSectionNames(
    const BlueConfigSection section) const
{
    return _impl->names[section];
}

const std::string& BlueConfig::get(const BlueConfigSection section,
                                   const std::string& sectionName,
                                   const std::string& key) const
{
    return _impl->get(section, sectionName, key);
}

brion::Targets BlueConfig::getTargets() const
{
    Targets targets;
    const URIs& uris = getTargetSources();
    BOOST_FOREACH (const URI& uri, uris)
        targets.push_back(Target(uri.getPath()));
    return targets;
}

URI BlueConfig::getCircuitSource() const
{
    const std::string& path =
        get(CONFIGSECTION_RUN, _impl->getRun(), BLUECONFIG_CIRCUIT_PATH_KEY);
    const std::string filename =
        path + (fs::exists(fs::path(path) / CIRCUIT_FILE_MVD3)
                    ? CIRCUIT_FILE_MVD3
                    : CIRCUIT_FILE_MVD2);

    URI uri;
    uri.setScheme("file");
    uri.setPath(filename);
    return uri;
}

URI BlueConfig::getSynapseSource() const
{
    URI uri;
    uri.setScheme("file");
    uri.setPath(
        get(CONFIGSECTION_RUN, _impl->getRun(), BLUECONFIG_NRN_PATH_KEY));
    return uri;
}

URI BlueConfig::getMorphologySource() const
{
    URI uri;
    uri.setScheme("file");
    std::string bare =
        get(CONFIGSECTION_RUN, _impl->getRun(), BLUECONFIG_MORPHOLOGY_PATH_KEY);
    const fs::path barePath(bare);
    const fs::path guessedPath = barePath / MORPHOLOGY_HDF5_FILES_SUBDIRECTORY;
    if (fs::exists(guessedPath) && fs::is_directory(guessedPath))
        uri.setPath(guessedPath.string());
    else
        uri.setPath(bare);

    return uri;
}

URI BlueConfig::getReportSource(const std::string& report) const
{
    std::string format =
        get(CONFIGSECTION_REPORT, report, BLUECONFIG_REPORT_FORMAT_KEY);
    if (format.empty())
    {
        LBWARN << "Invalid or missing report  " << report << std::endl;
        return URI();
    }

    boost::algorithm::to_lower(format);

    if (format == "binary" || format == "bin")
        return URI(std::string("file://") + _impl->getOutputRoot() + "/" +
                   report + ".bbp");

    if (format == "hdf5" || format.empty() || fs::is_directory(format))
        return URI(std::string("file://") + _impl->getOutputRoot() + "/" +
                   report + ".h5");

    return URI(_impl->getOutputRoot());
}

URI BlueConfig::getSpikeSource() const
{
    std::string path =
        get(CONFIGSECTION_RUN, _impl->getRun(), BLUECONFIG_SPIKES_PATH_KEY);
    if (path.empty())
        path = _impl->getOutputRoot() + SPIKE_FILE;
    URI uri;
    uri.setScheme("file");
    uri.setPath(path);
    return uri;
}

brion::URIs BlueConfig::getTargetSources() const
{
    const std::string& run = _impl->getRun();

    URIs uris;
    const std::string& nrnPath =
        get(brion::CONFIGSECTION_RUN, run, BLUECONFIG_NRN_PATH_KEY);
    if (!nrnPath.empty())
    {
        URI uri;
        uri.setScheme("file");
        uri.setPath(nrnPath + "/" + CIRCUIT_TARGET_FILE);
        uris.push_back(uri);
    }

    const std::string& targetPath =
        get(brion::CONFIGSECTION_RUN, run, BLUECONFIG_TARGET_FILE_KEY);
    if (!targetPath.empty())
    {
        URI uri;
        uri.setScheme("file");
        uri.setPath(targetPath);
        uris.push_back(uri);
    }

    return uris;
}

std::string BlueConfig::getCircuitTarget() const
{
    return _impl->getCircuitTarget();
}

GIDSet BlueConfig::parseTarget(const std::string& target) const
{
    return brion::Target::parse(getTargets(), target);
}

float BlueConfig::getTimestep() const
{
    const std::string& run = _impl->getRun();
    float timestep = std::numeric_limits<float>::quiet_NaN();
    _impl->get<float>(brion::CONFIGSECTION_RUN, run, BLUECONFIG_DT_KEY,
                      timestep);
    return timestep;
}

std::ostream& operator<<(std::ostream& os, const BlueConfig& config)
{
    for (size_t i = 0; i < CONFIGSECTION_ALL; ++i)
    {
        BOOST_FOREACH (const ValueTable::value_type& entry,
                       config._impl->table[i])
        {
            os << boost::lexical_cast<std::string>(BlueConfigSection(i)) << " "
               << entry.first << std::endl;
            BOOST_FOREACH (const KVStore::value_type& pair, entry.second)
            {
                os << "   " << pair.first << " " << pair.second << std::endl;
            }
            os << std::endl;
        }
    }

    return os;
}
}
