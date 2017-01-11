/* Copyright (c) 2014-2016, EPFL/Blue Brain Project
 *                          Juan Hernando Vieites <jhernando@fi.upm.es>
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

#include "spikeReport.h"

#include "spikeReportPlugin.h"
#include "pluginInitData.h"
#include <brion/version.h>

#include <lunchbox/plugin.h>
#include <lunchbox/pluginFactory.h>
#include <lunchbox/uint128_t.h>
#include <boost/scoped_ptr.hpp>

namespace
{
const std::string spikePluginDSONamePattern( "Brion.*SpikeReport" );
}

namespace brion
{
namespace
{
using SpikePluginFactory = lunchbox::PluginFactory<SpikeReportPlugin>;

class PluginLoader
{
public:
    PluginLoader()
    {
        SpikePluginFactory::getInstance().load( BRION_VERSION_ABI,
                                                lunchbox::getLibraryPaths(),
                                                spikePluginDSONamePattern );
    }
};

void _loadPlugins()
{
    static PluginLoader loader; // Use static class instantion for thread-safety
}
}

namespace detail
{
class SpikeReport
{
public:
    explicit SpikeReport( const SpikeReportInitData& initData )
    {
        _loadPlugins();
        plugin.reset( SpikePluginFactory::getInstance().create( initData ) );
    }

    boost::scoped_ptr< SpikeReportPlugin > plugin;
};
}

SpikeReport::SpikeReport( const URI& uri, const int mode )
    : _impl( new detail::SpikeReport( SpikeReportInitData( uri, mode )))
{
}

SpikeReport::~SpikeReport()
{
    close();
    delete _impl;
}

std::string SpikeReport::getDescriptions()
{
    _loadPlugins();
    return SpikePluginFactory::getInstance().getDescriptions();
}

const URI& SpikeReport::getURI() const
{
    return _impl->plugin->getURI();
}

SpikeReport::ReadMode SpikeReport::getReadMode() const
{
    return _impl->plugin->getReadMode();
}

float SpikeReport::getStartTime() const
{
    return _impl->plugin->getStartTime();
}

float SpikeReport::getEndTime() const
{
    return _impl->plugin->getEndTime();
}

const Spikes& SpikeReport::getSpikes() const
{
    return _impl->plugin->getSpikes();
}

void SpikeReport::writeSpikes( const Spikes& spikes )
{
    _impl->plugin->writeSpikes( spikes );
}

bool SpikeReport::waitUntil( const float timeStamp, const uint32_t timeout )
{
    return _impl->plugin->waitUntil( timeStamp, timeout );
}

float SpikeReport::getNextSpikeTime()
{
    return _impl->plugin->getNextSpikeTime();
}

float SpikeReport::getLatestSpikeTime()
{
    return _impl->plugin->getLatestSpikeTime();
}

void SpikeReport::clear( const float startTime, const float endTime )
{
    _impl->plugin->clear( startTime, endTime );
}

void SpikeReport::close()
{
    _impl->plugin->close();
}

}
