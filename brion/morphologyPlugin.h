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

#include <brion/api.h>
#include <brion/morphology.h>     // Needed by doxygen
#include <brion/pluginInitData.h> // base class
#include <brion/types.h>
#include <servus/serializable.h> // base class

namespace brion
{
/**
 * Basic plugin init data for MorphologyPlugin.
 * @version 1.4
 */
class MorphologyInitData : public PluginInitData
{
public:
    explicit MorphologyInitData(
        const URI& uri, const MorphologyVersion v = MORPHOLOGY_VERSION_H5_1_1,
        const CellFamily f = FAMILY_NEURON)
        : PluginInitData(uri, MODE_READ)
        , version(v)
        , family(f)
    {
        if (f == FAMILY_GLIA && v != MORPHOLOGY_VERSION_H5_1_1)
            throw std::runtime_error(
                "Glia cells only support HDF5 version 1.1");
    }

    MorphologyInitData(const URI& uri, const MorphologyVersion v,
                       const unsigned int accessMode)
        : PluginInitData(uri, accessMode)
        , version(v)
        , family(FAMILY_NEURON)
    {
    }

    MorphologyInitData(const URI& uri, const CellFamily f)
        : PluginInitData(uri, MODE_WRITE)
        , version(MORPHOLOGY_VERSION_H5_1_1)
        , family(f)
    {
    }

    MorphologyVersion version;
    CellFamily family;
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
class MorphologyPlugin : public servus::Serializable
{
public:
    /** @internal Needed by the PluginRegisterer. */
    using InterfaceT = MorphologyPlugin;

    /** @internal Needed by the PluginRegisterer. */
    using InitDataT = MorphologyInitData;

    MorphologyPlugin(const InitDataT& data)
        : _data(data)
    {
    }

    virtual ~MorphologyPlugin() {}
    /** Load all data of the morphology.
     *  Needs to be thread-safe wrt other instances. May throw std::exception.
     */
    virtual void load() = 0;

    /** @internal */
    InitDataT& getInitData() { return _data; }
    const InitDataT& getInitData() const { return _data; }
    /** @copydoc brion::Morphology::getCellFamily */
    CellFamily getCellFamily() const { return _data.family; }
    /** @internal */
    MorphologyVersion getVersion() const { return _data.version; }
    /** @copydoc brion::Morphology::getPoints */
    Vector4fs& getPoints() { return _points; }
    const Vector4fs& getPoints() const { return _points; }
    /** @copydoc brion::Morphology::getSections */
    Vector2is& getSections() { return _sections; }
    const Vector2is& getSections() const { return _sections; }
    /** @copydoc brion::Morphology::getSectionTypes */
    SectionTypes& getSectionTypes() { return _sectionTypes; }
    const SectionTypes& getSectionTypes() const { return _sectionTypes; }
    /** @copydoc brion::Morphology::getPerimeters */
    floats& getPerimeters() { return _perimeters; }
    const floats& getPerimeters() const { return _perimeters; }
protected:
    InitDataT _data;
    Vector4fs _points;
    Vector2is _sections;
    SectionTypes _sectionTypes;
    floats _perimeters;

    // Serializable API
    std::string getTypeName() const final { return "brion::MorphologyPlugin"; }
    bool _fromBinary(const void* data, const size_t size) final;
    servus::Serializable::Data _toBinary() const final;
};
}

namespace std
{
inline string to_string(const brion::MorphologyInitData& data)
{
    return to_string(data.getURI());
}
}

#include "morphologyPlugin.ipp" // inline impl to allow header-only usage
