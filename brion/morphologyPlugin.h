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

#ifndef BRION_MORPHOLOGYPLUGIN_H
#define BRION_MORPHOLOGYPLUGIN_H

#include <brion/api.h>
#include <brion/morphology.h> // Needed by doxygen
#include <brion/pluginInitData.h>
#include <brion/types.h>

namespace brion
{
/**
 * Basic plugin init data for MorphologyPlugin.
 * @version 1.4
 */
class MorphologyInitData : public PluginInitData
{
public:
    explicit MorphologyInitData(const URI& uri)
        : PluginInitData(uri, MODE_READ)
        , version(MORPHOLOGY_VERSION_H5_1_1)
        , family(FAMILY_NEURON)
    {
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
class MorphologyPlugin
{
public:
    /** @internal Needed by the PluginRegisterer. */
    typedef MorphologyPlugin InterfaceT;

    /** @internal Needed by the PluginRegisterer. */
    typedef MorphologyInitData InitDataT;

    MorphologyPlugin(const MorphologyInitData& data)
        : _data(data)
    {
    }

    virtual ~MorphologyPlugin() {}
    /** @name Read API */
    //@{
    /** @copydoc brion::Morphology::getCellFamily */
    CellFamily getCellFamily() const { return _data.family; }
    /** @copydoc brion::Morphology::getVersion */
    MorphologyVersion getVersion() const { return _data.version; }
    /** @copydoc brion::Morphology::readPoints */
    virtual Vector4fsPtr readPoints() const = 0;

    /** @copydoc brion::Morphology::readSections */
    virtual Vector2isPtr readSections() const = 0;

    /** @copydoc brion::Morphology::readSectionTypes */
    virtual SectionTypesPtr readSectionTypes() const = 0;

    /** @copydoc brion::Morphology::readApicals */
    virtual Vector2isPtr readApicals() const = 0;

    /** @copydoc brion::Morphology::readPerimeters */
    virtual floatsPtr readPerimeters() const = 0;

    //@}

    /** @name Write API */
    //@{
    /** @copydoc brion::Morphology::writePoints */
    virtual void writePoints(const Vector4fs& points) = 0;

    /** @copydoc brion::Morphology::writeSections */
    virtual void writeSections(const Vector2is& sections) = 0;

    /** @copydoc brion::Morphology::writeSectionTypes */
    virtual void writeSectionTypes(const SectionTypes& types) = 0;

    /** @copydoc brion::Morphology::writeApicals */
    virtual void writeApicals(const Vector2is& apicals) = 0;

    /** @copydoc brion::Morphology::writePerimeters */
    virtual void writePerimeters(const floats& perimeters) = 0;

    /** @copydoc brion::Morphology::flush */
    virtual void flush() = 0;
    //@}

protected:
    MorphologyInitData _data;
};
}

namespace std
{
inline string to_string(const brion::MorphologyInitData& data)
{
    return to_string(data.getURI());
}
}
#endif
