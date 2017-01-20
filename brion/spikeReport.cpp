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

#include "spikeReport.h"
#include "spikeReportPlugin.h"

#include "pluginInitData.h"

#include <brion/version.h>

#include <lunchbox/plugin.h>
#include <lunchbox/pluginFactory.h>
#include <lunchbox/threadPool.h>
#include <lunchbox/uint128_t.h>

#include <memory>

namespace
{
const std::string spikePluginDSONamePattern( "Brion.*SpikeReport" );
}

using SpikeReportInitData = ::brion::PluginInitData;

namespace brion
{
namespace
{
using SpikePluginFactory = lunchbox::PluginFactory< SpikeReportPlugin >;

class PluginLoader

{
public:
    PluginLoader()
    {
        SpikePluginFactory::getInstance().load(
            BRION_VERSION_ABI, lunchbox::getLibraryPaths(), spikePluginDSONamePattern );
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

public:
    std::unique_ptr< SpikeReportPlugin > plugin;
    lunchbox::ThreadPool threadPool{1};
};
}
}

namespace brion
{

SpikeReport::SpikeReport( const URI& uri, int mode )
    : _impl( new detail::SpikeReport( SpikeReportInitData( uri, mode ) ) )
{
    switch ( mode )
    {
    case MODE_READ:
    case MODE_WRITE:
        break;
    default:
        LBTHROW( std::runtime_error( "Unhandled open mode" ) );
    }
}

SpikeReport::SpikeReport( const URI& uri, const GIDSet& ids )
    : SpikeReport( uri, MODE_READ )
{
    _impl->plugin->setFilter( ids );
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

void SpikeReport::close()
{
    if ( _impl->plugin->isClosed() )
        return;

    if ( _impl->threadPool.hasPendingJobs() )
    {
        // interrupt the jobs
        _impl->plugin->_setInterrupted( true );
        // blocks until all the pending jobs are done
        _impl->threadPool.post( [] {} ).get();
    }
    _impl->plugin->close();
    _impl->plugin->_setClosed();
}

const URI& SpikeReport::getURI() const
{
    return _impl->plugin->getURI();
}

float SpikeReport::getCurrentTime() const
{
    return _impl->plugin->getCurrentTime();
}

SpikeReport::State SpikeReport::getState() const
{
    return _impl->plugin->getState();
}

bool SpikeReport::isClosed() const
{
    return _impl->plugin->isClosed();
}

void SpikeReport::interrupt()
{
    if ( _impl->threadPool.hasPendingJobs() )
    {
        _impl->plugin->_setInterrupted( true );
        // blocks until all the pending jobs are done
        _impl->threadPool.post( [] {} ).get();
        _impl->plugin->_setInterrupted( false );
    }
}

std::future< Spikes > SpikeReport::read( float min )
{
    _impl->plugin->_checkNotClosed();
    _impl->plugin->_checkCanRead();
    _impl->plugin->_checkStateOk();

    if ( min < getCurrentTime() )
    {
        LBTHROW( std::logic_error(
            "Can't read to a time stamp inferior to the current time" ) );
    }

    if ( _impl->threadPool.hasPendingJobs() )
    {
        LBTHROW( std::runtime_error( "Can't read : Pending read operation" ) );
    }

    return _impl->threadPool.post( [&, min] { return _impl->plugin->read( min ); } );
}

std::future< Spikes > SpikeReport::readUntil( const float max )
{
    _impl->plugin->_checkNotClosed();
    _impl->plugin->_checkCanRead();
    _impl->plugin->_checkStateOk();

    if ( max <= getCurrentTime() )
    {
        LBTHROW( std::logic_error(
            "Can't read to a time stamp inferior to the current time" ) );
    }

    if ( _impl->threadPool.hasPendingJobs() )
    {
        LBTHROW( std::runtime_error( "Can't read : Pending read operation" ) );
    }
    return _impl->threadPool.post( [&, max] { return _impl->plugin->readUntil( max ); } );
}

std::future< void > SpikeReport::seek( const float toTimeStamp )
{
    _impl->plugin->_checkNotClosed();
    if ( _impl->plugin->getAccessMode() == MODE_READ )
    {
        if ( _impl->threadPool.hasPendingJobs() )
        {
            LBTHROW( std::runtime_error( "Can't seek : Pending read operation" ) );
        }
        return _impl->threadPool.post(
            [&, toTimeStamp] { return _impl->plugin->readSeek( toTimeStamp ); } );
    }

    return _impl->threadPool.post(
        [&, toTimeStamp] { return _impl->plugin->writeSeek( toTimeStamp ); } );
}

void SpikeReport::write( const Spikes& spikes )
{
    _impl->plugin->_checkCanWrite();
    _impl->plugin->_checkNotClosed();

    if ( _impl->threadPool.hasPendingJobs() )
    {
        LBTHROW( std::runtime_error( "Can't write spikes : Pending seek operation" ) );
    }

    if ( spikes.size() && spikes.front().first < getCurrentTime() )
    {
        LBTHROW( std::logic_error( "Can't write spikes: first spike"
                                   " time inferior to the current time" ) );
    }

    _impl->plugin->write( spikes );
}
}
