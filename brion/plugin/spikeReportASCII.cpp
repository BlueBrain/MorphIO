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

#include "spikeReportASCII.h"

#include "../detail/skipWhiteSpace.h"
#include "../pluginInitData.h"

#include <boost/filesystem.hpp>
#include <boost/regex.hpp>

#include <lunchbox/pluginRegisterer.h>

#include <cmath>
#include <fstream>

namespace brion
{
namespace plugin
{
SpikeReportASCII::SpikeReportASCII(const SpikeReportInitData& initData)
    : SpikeReportPlugin(initData)
    , _lastReadPosition(_spikes.begin())
{
    // clear the file if it exists
    if (initData.getAccessMode() == MODE_WRITE &&
        boost::filesystem::exists(initData.getURI().getPath()))
    {
        std::ofstream ofs;
        ofs.open(initData.getURI().getPath(),
                 std::ofstream::out | std::ofstream::trunc);
        ofs.close();
    }
}

Spikes SpikeReportASCII::read(const float)
{
    // In file based reports, this function reads all remaining data.
    Spikes spikes;
    auto start = _lastReadPosition;
    _lastReadPosition = _spikes.end();
    _currentTime = UNDEFINED_TIMESTAMP;
    _state = State::ended;

    for (; start != _spikes.end(); ++start)
        pushBack(*start, spikes);

    return spikes;
}

Spikes SpikeReportASCII::readUntil(const float toTimeStamp)
{
    Spikes spikes;
    auto start = _lastReadPosition;

    _lastReadPosition =
        std::lower_bound(_lastReadPosition, _spikes.end(), toTimeStamp,
                         [](const Spike& spike, float val) {
                             return spike.first < val;
                         });

    if (_lastReadPosition != _spikes.end())
        _currentTime = _lastReadPosition->first;
    else
    {
        _currentTime = UNDEFINED_TIMESTAMP;
        _state = State::ended;
    }

    if (start != _spikes.end())
    {
        std::for_each(start, _lastReadPosition,
                      [&spikes, this](const Spike& spike) {
                          pushBack(spike, spikes);
                      });
    }
    return spikes;
}

void SpikeReportASCII::readSeek(const float toTimeStamp)
{
    if (_spikes.empty())
    {
        _currentTime = UNDEFINED_TIMESTAMP;
        _state = State::ended;
        return;
    }

    if (toTimeStamp < _spikes.begin()->first)
    {
        _lastReadPosition = _spikes.begin();
        _state = State::ok;
        _currentTime = toTimeStamp;
    }
    else if (toTimeStamp > _spikes.rbegin()->first)
    {
        _lastReadPosition = _spikes.end();
        _state = State::ended;
        _currentTime = brion::UNDEFINED_TIMESTAMP;
    }
    else
    {
        _lastReadPosition =
            std::lower_bound(_spikes.begin(), _spikes.end(), toTimeStamp,
                             [](const Spike& spike, float val) {
                                 return spike.first < val;
                             });
        _state = State::ok;
        _currentTime = toTimeStamp;
    }
}

void SpikeReportASCII::writeSeek(const float toTimeStamp)
{
    if (toTimeStamp < _currentTime)
        LBTHROW(
            std::runtime_error("Backward seek not supported in write mode"));

    _currentTime = toTimeStamp;
}

namespace
{
void _parse(Spikes& spikes, const std::string& filename,
            const std::function<bool(const std::string&, Spike&)>& parse)
{
    std::fstream file(filename.c_str(),
                      std::ios_base::binary | std::ios_base::in);

    size_t lineNumber = 0;
    file >> detail::SkipWhiteSpace(lineNumber);

#ifdef BRION_USE_OPENMP
    // The batch size is divisible by all reasonable core counts between 1
    // and 50.
    const size_t batchSize = 554400;

    while (!file.fail())
    {
        // Reading a batch of strings to parse in parallel
        Strings lines(batchSize);
        size_t linesRead = 0;

        while (linesRead != lines.size() &&
               !std::getline(file, lines[linesRead]).fail())
        {
            // This check skips comments
            const char c = lines[linesRead][0];
            if (c != '/' && c != '#')
                ++linesRead;
            size_t skipCount = 0;
            file >> detail::SkipWhiteSpace(skipCount);
        }

        const size_t first = spikes.size();
        spikes.resize(first + linesRead);

// Parsing strings
#pragma omp parallel for
        for (int64_t i = 0; i < int64_t(linesRead); ++i)
        {
            const std::string& line = lines[i];
            if (!parse(line.c_str(), spikes[first + i]))
            {
                // Getting the line number right is not possible unless the
                // line numbers are stored along with the strings.
                // Instead of telling the line number, the full line will be
                // printed. The error should be easy to spot in the file as
                // it will be found at the first occurence in the file.
                LBERROR << "Error reading spike times file: " << filename
                        << ", line: " << lines[i] << std::endl;
            }
        }
    }
#else
    std::string line;
    while (!std::getline(file, line).fail())
    {
        ++lineNumber;
        if (line[0] == '/' || line[0] == '#')
            continue;

        Spike spike;
        if (!parse(line.c_str(), spike))
        {
            LBTHROW(std::runtime_error("Parsing spike times file " + filename +
                                       " failed at line " +
                                       std::to_string(lineNumber)));
        }
        spikes.emplace_back(spike);

        file >> detail::SkipWhiteSpace(lineNumber);
    }
#endif

    if (file.fail() && !file.eof())
        LBTHROW(std::runtime_error("IO error reading spike times file: " +
                                   filename));
}
}

Spikes SpikeReportASCII::parse(const Strings& files, const ParseFunc& parse)
{
    Spikes spikes;
    for (const auto& file : files)
        _parse(spikes, file, parse);
    std::sort(spikes.begin(), spikes.end());
    return spikes;
}

Spikes SpikeReportASCII::parse(const std::string& filename,
                               const ParseFunc& parse)
{
    Spikes spikes;
    _parse(spikes, filename, parse);
    std::sort(spikes.begin(), spikes.end());
    return spikes;
}

void SpikeReportASCII::append(const Spikes& spikes, const WriteFunc& writefunc)
{
    if (!spikes.size())
        return;

    std::fstream file{getURI().getPath(),
                      std::ios_base::binary | std::ios::out | std::ios::app};
    if (!file.is_open())
    {
        _state = State::failed;
        return;
    }

    for (const Spike& spike : spikes)
        writefunc(file, spike);

    file.flush();

    const float lastTimestamp = spikes.rbegin()->first;
    _currentTime =
        std::nextafter(lastTimestamp, std::numeric_limits<float>::max());
    _endTime = std::max(_endTime, lastTimestamp);
}
}
}
