/* Copyright (c) 2013-2016, EPFL/Blue Brain Project
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
#include <brion/types.h>
#include <brion/morphology.h> // Needed by doxygen
#include <brion/pluginInitData.h>

#include <boost/noncopyable.hpp>
#include <boost/lexical_cast.hpp>

namespace brion
{

/**
 * Basic plugin init data for MorphologyPlugin.
 * @version 1.4
 */
class MorphologyInitData : public PluginInitData
{
public:
    explicit MorphologyInitData( const URI& uri )
        : PluginInitData( uri, MODE_READ )
        , _version( MORPHOLOGY_VERSION_H5_1_1 )
        , _family( FAMILY_NEURON )
    {}

    MorphologyInitData( const URI& uri,
                        const MorphologyVersion version,
                        const unsigned int accessMode )
        : PluginInitData( uri, accessMode )
        , _version( version )
        , _family( FAMILY_NEURON )
    {}

    MorphologyInitData( const URI& uri,
                        const CellFamily family )
        : PluginInitData( uri, MODE_WRITE )
        , _version( MORPHOLOGY_VERSION_H5_1_1 )
        , _family( family )
    {}

    MorphologyVersion getVersion() const
    {
        return _version;
    }

    CellFamily getFamily() const
    {
        return _family;
    }

protected:
    const MorphologyVersion _version;
    const CellFamily _family;
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
class MorphologyPlugin : public boost::noncopyable
{
public:
    /** @internal Needed by the PluginRegisterer. */
    typedef MorphologyPlugin InterfaceT;

    /** @internal Needed by the PluginRegisterer. */
    typedef MorphologyInitData InitDataT;

    /** @internal */
    virtual ~MorphologyPlugin() {}

    /** @name Read API */
    //@{
    /** @copydoc brion::Morphology::getCellFamily */
    virtual CellFamily getCellFamily() const = 0;

    /** @copydoc brion::Morphology::readPoints */
    virtual Vector4fsPtr readPoints( MorphologyRepairStage stage ) const = 0;

    /** @copydoc brion::Morphology::readSections */
    virtual Vector2isPtr readSections( MorphologyRepairStage stage ) const = 0;

    /** @copydoc brion::Morphology::readSectionTypes */
    virtual SectionTypesPtr readSectionTypes() const = 0;

    /** @copydoc brion::Morphology::readApicals */
    virtual Vector2isPtr readApicals() const = 0;

    /** @copydoc brion::Morphology::readPerimeters */
    virtual floatsPtr readPerimeters() const = 0;

    virtual MorphologyVersion getVersion() const = 0;
    //@}

    /** @name Write API */
    //@{
    /** @copydoc brion::Morphology::writePoints */
    virtual void writePoints( const Vector4fs& points,
                              MorphologyRepairStage stage ) = 0;

    /** @copydoc brion::Morphology::writeSections */
    virtual void writeSections( const Vector2is& sections,
                                MorphologyRepairStage stage ) = 0;

    /** @copydoc brion::Morphology::writeSectionTypes */
    virtual void writeSectionTypes( const SectionTypes& types ) = 0;

    /** @copydoc brion::Morphology::writeApicals */
    virtual void writeApicals( const Vector2is& apicals ) = 0;

    /** @copydoc brion::Morphology::writePerimeters */
    virtual void writePerimeters( const floats& perimeters ) = 0;

    /** @copydoc brion::Morphology::flush */
    virtual void flush() = 0;
    //@}
};

}

namespace std
{
inline string to_string( const brion::MorphologyInitData& data )
{
    return to_string( data.getURI( ));
}
}
#endif
