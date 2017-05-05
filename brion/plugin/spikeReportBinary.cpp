/* Copyright (c) 2015-2017, EPFL/Blue Brain Project
 *                          Stefan.Eilemann@epfl.ch
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

#include "spikeReportBinary.h"

#include <lunchbox/memoryMap.h>
#include <lunchbox/pluginRegisterer.h>

#include <boost/filesystem.hpp>
#include <boost/filesystem/path.hpp>

#include <fstream>

namespace brion
{
namespace plugin
{
namespace
{
lunchbox::PluginRegisterer<SpikeReportBinary> registerer;
const char* const BINARY_REPORT_FILE_EXT = ".spikes";
}

namespace fs = boost::filesystem;

class Header
{
public:
    bool isValid() const { return _magic == 0xf0a && _version == 1; }
private:
    uint32_t _magic = 0xf0a;
    uint32_t _version = 1;
};

class BinaryReportMap
{
public:
    // Read-only mapping
    BinaryReportMap(const std::string& path)
        : _map(path)
    {
        const size_t totalSize = _map.getSize();
        if (totalSize < sizeof(Header) || (totalSize % sizeof(uint32_t)) != 0)
        {
            LBTHROW(std::runtime_error("Incompatible binary report: " + path));
        }

        if (!_map.getAddress<Header>()->isValid())
        {
            LBTHROW(std::runtime_error("Invalid binary spike report header: " +
                                       path));
        }
    }

    // read-write mapping
    BinaryReportMap(const std::string& path, size_t nSpikes)
        : _map(path, sizeof(Header) + sizeof(Spike) * nSpikes)
    {
        *(_map.getAddress<Header>()) = Header();
    }

    void resize(const size_t nSpikes)
    {
        _map.resize(sizeof(Header) + sizeof(Spike) * nSpikes);
    }

    size_t getNumSpikes() const
    {
        return (_map.getSize() - sizeof(Header)) / sizeof(Spike);
    }

    const Spike* getReadableSpikes() const
    {
        return reinterpret_cast<const Spike*>(_map.getAddress<uint8_t>() +
                                              sizeof(Header));
    }

    Spike* getWritableSpikes()
    {
        return reinterpret_cast<Spike*>(_map.getAddress<uint8_t>() +
                                        sizeof(Header));
    }

private:
    lunchbox::MemoryMap _map;
};

SpikeReportBinary::SpikeReportBinary(const SpikeReportInitData& initData)
    : SpikeReportPlugin(initData)
{
    if (_accessMode == MODE_READ)
        _memFile.reset(new BinaryReportMap(getURI().getPath()));
    else
        _memFile.reset(new BinaryReportMap(getURI().getPath(), 0));

    const Spike* spikeArray = _memFile->getReadableSpikes();
    const size_t nElems = _memFile->getNumSpikes();
    if (nElems != 0)
        _endTime = spikeArray[nElems - 1].first;
}

bool SpikeReportBinary::handles(const SpikeReportInitData& initData)
{
    const URI& uri = initData.getURI();
    if (!uri.getScheme().empty() && uri.getScheme() != "file")
        return false;

    const auto ext = boost::filesystem::path(uri.getPath()).extension();
    return ext == brion::plugin::BINARY_REPORT_FILE_EXT;
}

std::string SpikeReportBinary::getDescription()
{
    return "Blue Brain binary spike reports: "
           "[file://]/path/to/report" +
           std::string(BINARY_REPORT_FILE_EXT);
}

Spikes SpikeReportBinary::read(const float)
{
    // In file based reports, this function reads all remaining data.
    Spikes spikes;
    const Spike* spikeArray = _memFile->getReadableSpikes();
    const size_t nElems = _memFile->getNumSpikes();

    for (; _startIndex < nElems; ++_startIndex)
        pushBack(spikeArray[_startIndex], spikes);

    _currentTime = UNDEFINED_TIMESTAMP;
    _state = State::ended;
    return spikes;
}

Spikes SpikeReportBinary::readUntil(const float max)
{
    Spikes spikes;

    const Spike* spikeArray = _memFile->getReadableSpikes();
    const size_t nElems = _memFile->getNumSpikes();

    for (; _startIndex < nElems; ++_startIndex)
    {
        if (spikeArray[_startIndex].first >= max)
        {
            _currentTime = spikeArray[_startIndex].first;
            break;
        }
        pushBack(spikeArray[_startIndex], spikes);
    }

    if (_startIndex == nElems)
    {
        _currentTime = UNDEFINED_TIMESTAMP;
        _state = State::ended;
    }

    return spikes;
}

void SpikeReportBinary::readSeek(const float toTimeStamp)
{
    const Spike* spikeArray = _memFile->getReadableSpikes();
    const size_t nElems = _memFile->getNumSpikes();

    const Spike* position = nullptr;

    if (toTimeStamp < _currentTime)
    {
        position =
            std::lower_bound(spikeArray, spikeArray + _startIndex, toTimeStamp,
                             [](const Spike& spike, const float val) {
                                 return spike.first < val;
                             });
    }
    else
    {
        position = std::lower_bound(spikeArray + _startIndex,
                                    spikeArray + nElems, toTimeStamp,
                                    [](const Spike& spike, const float val) {
                                        return spike.first < val;
                                    });
    }

    if (position == (spikeArray + nElems)) // end
    {
        _startIndex = nElems;
        _state = State::ended;
        _currentTime = UNDEFINED_TIMESTAMP;
    }
    else
    {
        _state = State::ok;
        _startIndex = std::distance(spikeArray, position);
        _currentTime = toTimeStamp;
    }
}

void SpikeReportBinary::writeSeek(float toTimeStamp)
{
    readSeek(toTimeStamp);
}

void SpikeReportBinary::write(const Spike* spikes, const size_t size)
{
    size_t totalSpikes = _startIndex + size;

    if (size == 0)
        return;

    // create or resize the file
    if (_memFile->getNumSpikes() != totalSpikes)
        _memFile->resize(totalSpikes);

    Spike* spikeArray = _memFile->getWritableSpikes();
    for (size_t i = 0; i != size; ++i)
        spikeArray[_startIndex++] = spikes[i];

    const float lastTimestamp = spikes[size - 1].first;
    _currentTime =
        std::nextafter(lastTimestamp, std::numeric_limits<float>::max());
    _endTime = std::max(_endTime, lastTimestamp);
}
}
} // namespaces
