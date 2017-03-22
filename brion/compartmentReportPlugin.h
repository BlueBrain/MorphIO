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

#pragma once

#include <brion/api.h>
#include <brion/pluginInitData.h>
#include <brion/types.h>

#include <boost/noncopyable.hpp>
#include <lunchbox/log.h>

namespace brion
{
/**
 * Basic plugin init data for CompartmentReportPlugin.
 * @version 1.5
 */
class CompartmentReportInitData : public PluginInitData
{
public:
    /** Create a CompartmentReportInitData object given a URI, access mode
     *  and GIDs.
     *
     * @param uri URI to compartment report. The report type is deduced from
     *        here.
     * @param accessMode the brion::AccessMode bitmask
     * @param gids the neurons of interest in READ_MODE
     * @version 1.4
     */
    explicit CompartmentReportInitData(const URI& uri,
                                       const int accessMode = MODE_READ,
                                       const GIDSet& gids = GIDSet())
        : PluginInitData(uri, accessMode)
        , _gids(gids)
    {
    }

    /** @return Returns the gids. @version 1.4 */
    const GIDSet& getGids() const { return _gids; }
private:
    const GIDSet _gids;
};

/**
 * Base interface for compartment report readers and writers.
 *
 * The following example creates a new report, initializes and registers it:
 * @code
 * class MyReport : CompartmentReportPlugin
 * {
 *     MyReport( const CompartmentReportInitData& pluginInitData );
 *     static bool handles( const CompartmentReportInitData& pluginInitData );
 *     ...
 * };
 * ...
 * // in the .cpp file
 * namespace
 * {
 *     lunchbox::PluginRegisterer< MyReport > registerer;
 * }
 * @endcode
 *
 * @version 1.4
 */
class CompartmentReportPlugin : public boost::noncopyable
{
public:
    /** @internal Needed by the PluginRegisterer. */
    typedef CompartmentReportPlugin InterfaceT;

    /** @internal Needed by the PluginRegisterer. */
    typedef CompartmentReportInitData InitDataT;

    /** @internal */
    virtual ~CompartmentReportPlugin() {}
    /** @name Abstract interface */
    //@{
    /** @copydoc brion::CompartmentReport::getStartTime */
    virtual float getStartTime() const = 0;

    /** @copydoc brion::CompartmentReport::getEndTime */
    virtual float getEndTime() const = 0;

    /** @copydoc brion::CompartmentReport::getTimestep */
    virtual float getTimestep() const = 0;

    /** @copydoc brion::CompartmentReport::getDataUnit */
    virtual const std::string& getDataUnit() const = 0;

    /** @copydoc brion::CompartmentReport::getTimeUnit */
    virtual const std::string& getTimeUnit() const = 0;

    /** @copydoc brion::CompartmentReport::getGIDs */
    virtual const GIDSet& getGIDs() const = 0;

    /** @copydoc brion::CompartmentReport::getOffsets */
    virtual const SectionOffsets& getOffsets() const = 0;

    /** @copydoc brion::CompartmentReport::getNumCompartments */
    virtual size_t getNumCompartments(size_t index) const = 0;

    /** @copydoc brion::CompartmentReport::getCompartmentCounts */
    virtual const CompartmentCounts& getCompartmentCounts() const = 0;

    /** @copydoc brion::CompartmentReport::getFrameSize */
    virtual size_t getFrameSize() const = 0;

    /** @copydoc brion::CompartmentReport::loadFrame */
    virtual floatsPtr loadFrame(float timestamp) const = 0;

    /** @copydoc brion::CompartmentReport::loadNeuron */
    virtual floatsPtr loadNeuron(uint32_t gid LB_UNUSED) const
    {
        throw std::runtime_error("loadNeuron() not implemented");
    }

    /** @copydoc brion::CompartmentReport::updateMapping */
    virtual void updateMapping(const GIDSet& gids) = 0;

    /** @copydoc brion::CompartmentReport::setBufferSize */
    virtual void setBufferSize(size_t size) { /* To keep doxygen happy */ (void)size; }

    /** @copydoc brion::CompartmentReport::clearBuffer */
    virtual void clearBuffer() {}
    /** @copydoc brion::CompartmentReport::getBufferSize */
    virtual size_t getBufferSize() const { return 0; }
    /** @copydoc brion::CompartmentReport::writeHeader */
    virtual void writeHeader(float startTime, float endTime, float timestep,
                             const std::string& dunit,
                             const std::string& tunit) = 0;

    /** @copydoc brion::CompartmentReport::writeCompartments */
    virtual bool writeCompartments(uint32_t gid, const uint16_ts& counts) = 0;

    /** @copydoc brion::CompartmentReport::writeFrame */
    virtual bool writeFrame(uint32_t gid, const float* values, size_t size,
                            float timestamp) = 0;

    /** @copydoc brion::CompartmentReport::flush */
    virtual bool flush() = 0;

    /** @copydoc brion::CompartmentReport::erase */
    virtual bool erase() { return false; }
    //@}

    /** @copydoc brion::CompartmentReport::getIndex */
    size_t getIndex(const uint32_t gid) const
    {
        const auto& gids = getGIDs();
        const size_t index = std::distance(gids.begin(), gids.find(gid));
        if (index >= gids.size())
            LBTHROW(std::runtime_error("Gid " + std::to_string(gid) +
                                       " not in report"));
        return index;
    }
};
}

namespace std
{
inline string to_string(const brion::CompartmentReportInitData& data)
{
    return to_string(data.getURI());
}
}
