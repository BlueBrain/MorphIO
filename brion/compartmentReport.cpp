/* Copyright (c) 2013-2016, EPFL/Blue Brain Project
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

#include "compartmentReport.h"
#include "compartmentReportPlugin.h"

#include <lunchbox/log.h>
#include <lunchbox/plugin.h>
#include <lunchbox/pluginFactory.h>
#include <boost/filesystem.hpp>
#include <boost/scoped_ptr.hpp>

namespace brion
{
namespace detail
{

class CompartmentReport
{
public:
    typedef lunchbox::PluginFactory< CompartmentReportPlugin >
        CompartmentPluginFactory;

    explicit CompartmentReport( const CompartmentReportInitData& initData )
        : plugin( CompartmentPluginFactory::getInstance().create( initData ))
    {}

    const boost::scoped_ptr< CompartmentReportPlugin > plugin;

};
}

CompartmentReport::CompartmentReport( const URI& uri, const int mode,
                                      const GIDSet& gids )
    : _impl( new detail::CompartmentReport(
                 CompartmentReportInitData( uri, mode, gids )))
{}

CompartmentReport::~CompartmentReport()
{
    delete _impl;
}

const GIDSet& CompartmentReport::getGIDs() const
{
    return _impl->plugin->getGIDs();
}

const SectionOffsets& CompartmentReport::getOffsets() const
{
    return _impl->plugin->getOffsets();
}

const CompartmentCounts& CompartmentReport::getCompartmentCounts() const
{
    return _impl->plugin->getCompartmentCounts();
}

size_t CompartmentReport::getNumCompartments( const size_t index ) const
{
    return _impl->plugin->getNumCompartments( index );
}

float CompartmentReport::getStartTime() const
{
    return _impl->plugin->getStartTime();
}

float CompartmentReport::getEndTime() const
{
    return _impl->plugin->getEndTime();
}

float CompartmentReport::getTimestep() const
{
    return _impl->plugin->getTimestep();
}

const std::string& CompartmentReport::getDataUnit() const
{
    return _impl->plugin->getDataUnit();
}

const std::string& CompartmentReport::getTimeUnit() const
{
    return _impl->plugin->getTimeUnit();
}

size_t CompartmentReport::getFrameSize() const
{
    return _impl->plugin->getFrameSize();
}

size_t CompartmentReport::getBufferSize() const
{
    return _impl->plugin->getBufferSize();
}

floatsPtr CompartmentReport::loadFrame( const float timestamp ) const
{
    if( timestamp < getStartTime() || timestamp > getEndTime( ))
        return floatsPtr();

    return _impl->plugin->loadFrame( timestamp );
}

void CompartmentReport::setBufferSize( const size_t size )
{
    _impl->plugin->setBufferSize( size );
}

void CompartmentReport::clearBuffer()
{
    _impl->plugin->clearBuffer();
}

void CompartmentReport::updateMapping( const GIDSet& gids )
{
    _impl->plugin->updateMapping( gids );
}

void CompartmentReport::writeHeader( const float startTime, const float endTime,
                                     const float timestep,
                                     const std::string& dunit,
                                     const std::string& tunit )
{
    _impl->plugin->writeHeader( startTime, endTime, timestep, dunit, tunit );
}

bool CompartmentReport::writeCompartments( const uint32_t gid,
                                           const uint16_ts& counts )
{
    return _impl->plugin->writeCompartments( gid, counts );
}

bool CompartmentReport::writeFrame( const uint32_t gid, const floats& voltages,
                                    const float timestamp )
{
    return _impl->plugin->writeFrame( gid, voltages, timestamp );
}

bool CompartmentReport::flush()
{
    return _impl->plugin->flush();
}

}
