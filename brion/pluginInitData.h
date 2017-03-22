/* Copyright (c) 2013-2017, EPFL/Blue Brain Project
 *                          Ahmet Bilgili <ahmet.bilgili@epfl.ch>
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

#include <brion/types.h>

namespace brion
{
/**
 * Basic plugin init data for Brion plugin classes. SpikeReportInitData and
 * CompartmentReportInitData classes extend this class to initialize the
 * plugins through the factory.
 *
 * With the help of the PluginInitData class, the plugins can do the plugin
 * initialization in their constructor.
 *
 * @version 1.4
 */
class PluginInitData
{
public:
    /**
     * Create a PluginInitData object given a URI and access mode.
     *
     * @param uri URI to plugin. The report type is deduced from  here.
     *
     * @param accessMode the brion::AccessMode bitmask
     * @version 1.4
     */
    explicit PluginInitData(const URI& uri, const int accessMode = MODE_READ)
        : _uri(uri)
        , _accessMode(accessMode)
    {
    }

    /** @return the URI. @version 1.4 */
    const URI& getURI() const { return _uri; }
    /**
     *
     * @return Returns the access mode.
     * @version 1.4
     */
    int getAccessMode() const { return _accessMode; }
protected:
    const URI _uri;
    const int _accessMode;
};
}
