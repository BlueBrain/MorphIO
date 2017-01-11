/* Copyright (c) 2013-2017, EPFL/Blue Brain Project
 *                          Juan Hernando <jhernando@fi.upm.es>
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

#ifndef BRION_PLUGIN_SPIKEREPORTSIMPLESTREAMER_H
#define BRION_PLUGIN_SPIKEREPORTSIMPLESTREAMER_H

#include <brion/spikeReportPlugin.h>

#include <lunchbox/monitor.h>
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>

namespace brion
{
namespace plugin
{

/** A simple spike report streamer.

    This is an example implementation of spike streaming. This reader
    provides on demand loading of NEST reports.
*/
class SpikeReportSimpleStreamer : public SpikeReportPlugin
{
public:
    explicit SpikeReportSimpleStreamer( const SpikeReportInitData& initData );
    virtual ~SpikeReportSimpleStreamer();

    /** Check if this plugin can handle the given uri. */
    static bool handles( const SpikeReportInitData& initData );
    static std::string getDescription();

    const URI& getURI() const final;

    float getStartTime() const final;

    float getEndTime() const final;

    const Spikes& getSpikes() const final;

    SpikeReport::ReadMode getReadMode() const final
    {
        return SpikeReport::STREAM;
    }

    bool waitUntil( const float timeStamp, const uint32_t timeout ) final;

    float getNextSpikeTime() final;

    float getLatestSpikeTime() final;

    void clear( const float starTime, const float endTime ) final;

    void close() final;

private:
    const URI _uri;

    // This is the data set that is exposed to the user. This data set is
    // updated with cached incoming spikes by waitUntil.
    Spikes _spikes;

    Spikes _incoming;
    // Protects access to _incomingSpikes.
    mutable boost::mutex _mutex;
    lunchbox::Monitor<float> _lastTimeStamp;
    float _lastEndTime;

    boost::thread _readThread;

    void _readLoop();
};

}
}
#endif
