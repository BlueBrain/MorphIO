/* Copyright (c) 2013-2016, EPFL/Blue Brain Project
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

#ifndef SPIKEREPORTPLUGIN_H
#define SPIKEREPORTPLUGIN_H

#include <brion/api.h>
#include <brion/types.h>
#include <brion/spikeReport.h>
#include <brion/pluginInitData.h>
#include <lunchbox/debug.h>  // LBTHROW

#include <boost/noncopyable.hpp>

namespace brion
{
/**
 * Basic plugin init data for SpikeReportPlugin.
 * @version 1.4
 */
typedef PluginInitData SpikeReportInitData;

/** Base interface for spike report readers plugins.
 *
 * The following example creates a new plugin and registers it:
 * @code
 * class MyReport : SpikeReportPlugin
 * {
 *     MyReport( const SpikeReportInitData& initData );
 *     static bool handles( const SpikeReportInitData& initData );
 *     ...
 * };
 * ...
 * // in the .cpp file
 * namespace
 * {
 *     PluginRegisterer< MyReport > registerer;
 * }
 * @endcode
 *
 * Plugins can also be provided by shared libraries discovered at runtime.
 *
 * In this case, the registration must be occur from an extern C function named
 * LunchboxPluginRegister():
 * @code
 * // in the .cpp file
 * extern "C" int LunchboxPluginGetVersion() { return BRION_VERSION_ABI; }
 * extern "C" bool LunchboxPluginRegister()
 * {
 *     PluginRegisterer< MyReport > registerer;
 *     return true;
 * }
 * @endcode
 *
 * Plugin libraries in the LD_LIBRARY_PATH will be automatically registered if
 * they provide the abovementioned C functions and follow the naming convention:
 * \<lib_prefix\>Brion\<MyReportName\>SpikeReport.\<lib_extension\>
 *
 * @version 1.4
 */
class SpikeReportPlugin : public boost::noncopyable
{
public:
    /** @internal Needed by the PluginRegisterer. */
    typedef SpikeReportPlugin InterfaceT;

    /** @internal Needed by the PluginRegisterer. */
    typedef SpikeReportInitData InitDataT;

    /** @internal */
    virtual ~SpikeReportPlugin() {}

    /** @copydoc brion::SpikeReport::getStartTime */
    virtual float getStartTime() const
    {
       return UNDEFINED_TIMESTAMP;
    }

    /** @copydoc brion::SpikeReport::getEndTime */
    virtual float getEndTime() const
    {
       return UNDEFINED_TIMESTAMP;
    }

    /** @copydoc brion::SpikeReport::getSpikes */
    virtual const Spikes& getSpikes() const
    {
        LBTHROW( std::runtime_error(
                     "Operation not supported in spike report plugin" ));
    }

    /** @copydoc brion::SpikeReport::writeSpikes */
    virtual void writeSpikes( const Spikes& spikes LB_UNUSED )
    {
        LBTHROW( std::runtime_error(
                     "Operation not supported in spike report plugin" ));
    }

    /** @copydoc brion::SpikeReport::getURI */
    virtual const URI& getURI() const = 0;

    /** @copydoc brion::SpikeReport::getReadMode */
    virtual SpikeReport::ReadMode getReadMode() const = 0;

    /** @copydoc brion::SpikeReport::waitUntil */
    virtual bool waitUntil( const float timeStamp,
                            const uint32_t timeout LB_UNUSED )
    {
        return getEndTime() >= timeStamp;
    };

    /** @copydoc brion::SpikeReport::getNextSpikeTime */
    virtual float getNextSpikeTime() { return UNDEFINED_TIMESTAMP; }

    /** @copydoc brion::SpikeReport::getNextSpikeTime */
    virtual float getLatestSpikeTime() { return UNDEFINED_TIMESTAMP; }

    /** @copydoc brion::SpikeReport::clear */
    virtual void clear( const float startTime LB_UNUSED,
                        const float endTime LB_UNUSED ) { /* NOP */ }

    /** @copydoc brion::SpikeReport::close */
    virtual void close() { /* NOP */ }
};

}

namespace std
{
inline string to_string( const brion::SpikeReportInitData& data )
{
    return to_string( data.getURI( ));
}
}

#endif // SPIKEREPORTPLUGIN_H
