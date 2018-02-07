/* Copyright (c) 2013-2017, EPFL/Blue Brain Project
 *                          Juan Hernando <jhernando@fi.upm.es>
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

#include <api.h>
#include <morphology.h>     // Needed by doxygen
#include <types.h>

#include <string>

#include <properties.hpp>

namespace minimorph
{
/**
 * Basic plugin init data for MorphologyPlugin.
 * @version 1.4
 */
class MorphologyInitData
{
public:
    explicit MorphologyInitData(
        const URI& uri, const MorphologyVersion v = MORPHOLOGY_VERSION_H5_1_1,
        const CellFamily f = FAMILY_NEURON)
        : uri(uri)
        , version(v)
        , family(f)
    {
        if (f == FAMILY_GLIA && v != MORPHOLOGY_VERSION_H5_1_1)
            throw std::runtime_error(
                "Glia cells only support HDF5 version 1.1");
    }

    MorphologyInitData(const URI& uri, const MorphologyVersion v,
                       const unsigned int accessMode)
        : uri(uri)
        , version(v)
        , family(FAMILY_NEURON)
    {
    }

    MorphologyInitData(const URI& uri, const CellFamily f)
        : uri(uri)
        , version(MORPHOLOGY_VERSION_H5_1_1)
        , family(f)
    {
    }

    URI getURI() const{ return URI(uri); }

    MorphologyVersion version;
    CellFamily family;
    URI uri;
};

/**
 * Base interface for morphology readers plugins.
 *
 * The following example creates a new plugin and registers it:
 * @code
 * class MyMorphology : MorphologyPlugin
 * {
 *     MyMorphology( const MorphologyInitData& initData );
 *     static bool handles( const MorphologyInitData& initData );
 *     ...
 * };
 * ...
 * // in the .cpp file
 * namespace
 * {
 *     PluginRegisterer< MyMorphology > registerer;
 * }
 * @endcode
 *
 * @version 1.4
 */
class MorphologyPlugin
{
public:
    MorphologyPlugin(const MorphologyInitData& data)
        : _data(data)
    {
    }

    virtual ~MorphologyPlugin() {}
    /** Load all data of the morphology.
     *  Needs to be thread-safe wrt other instances. May throw std::exception.
     */
    virtual void load() = 0;

    /** @internal */
    MorphologyInitData& getInitData() { return _data; }
    const MorphologyInitData& getInitData() const { return _data; }

    /** @copydoc minimorph::Morphology::getCellFamily */
    CellFamily getCellFamily() const { return _data.family; }

    /** @internal */
    MorphologyVersion getVersion() const { return _data.version; }

    template <typename Property> std::vector<typename Property::Type>& get(){
        return _properties.get<Property>();
    }

    template <typename Property> const std::vector<typename Property::Type>& get() const {
        return _properties.get<Property>();
    }

protected:
    MorphologyInitData _data;
    Properties _properties;

    /*
    // Serializable API
    std::string getTypeName() const final { return "minimorph::MorphologyPlugin"; }
    bool _fromBinary(const void* data, const size_t size) final;
    */
};

}

// TODO: compile
#if 0
namespace std
{
inline string to_string(const minimorph::MorphologyInitData& data)
{
    return to_string(data.getURI());
}
}


#include "morphologyPlugin.ipp" // inline impl to allow header-only usage
#endif
