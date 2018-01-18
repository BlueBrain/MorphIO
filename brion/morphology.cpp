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

#include "morphology.h"

#include "morphologyPlugin.h"

#include "plugin/morphologyHDF5.h"
#include "plugin/morphologySWC.h"

#include <cassert>
/* todo: compile */
#include <iostream>

namespace brion
{
//TODO: compile
#if 0
namespace
{
/**
 * "Plugin" for copied and deserialized morphologies.
 *
 * Does not actually load any data, but holds the data gathered from the copy or
 * deserialization.
 */
class BinaryMorphology : public MorphologyPlugin
{
public:
    BinaryMorphology(const Morphology& from)
        : MorphologyPlugin(from.getInitData())
    {
        _points = from.getPoints();
        _sections = from.getSections();
        _sectionTypes = from.getSectionTypes();
        _perimeters = from.getPerimeters();
    }

    BinaryMorphology(const void* data, size_t size)
        : MorphologyPlugin(MorphologyInitData({}))
    {
      /*
        if (!fromBinary(data, size)){
          LBTHROW(std::runtime_error(
                  "Failed to construct morphology from binary data"));
        }
        */
    }

    void load() final { /*NOP*/}
};
}
#endif

class Morphology::Impl
{
public:
    explicit Impl(const URI& uri)
    {
      const size_t pos = uri.find_last_of(".");
      assert(pos != std::string::npos);
      if (uri.substr(pos) == ".h5") {
        plugin = std::unique_ptr<MorphologyPlugin>(
            new plugin::MorphologyHDF5(MorphologyInitData(uri)));
      }
      else if (uri.substr(pos) == ".swc") {
        plugin = std::unique_ptr<MorphologyPlugin>(
            new plugin::MorphologySWC(MorphologyInitData(uri)));
      }
      else {
        assert(false && "unhandled file type");
      }
    }

#if 0
    Impl(const Morphology& from)
        : plugin(new BinaryMorphology(from))
    {
    }

    Impl(const void* data, size_t size)
        : plugin(new BinaryMorphology(data, size))
    {
    }
#endif

    ~Impl()
    {
    }

    //TODO: Do we really need a PIMPL to hold the plugin?
    std::unique_ptr<MorphologyPlugin> plugin;
};

Morphology::Morphology(const URI& source)
    : _impl(new Impl(source))
{
}

#if 0
Morphology::Morphology(const void* data, size_t size)
    : _impl(new Impl(data, size))
{
}

Morphology::Morphology(const Morphology& from)
    : _impl(new Impl(from))
{
}
#endif

/*
Morphology& Morphology::operator=(const Morphology& from)
{
    if (this != &from)
        _impl.reset(new Impl(from));
    return *this;
}
*/

Morphology::Morphology(Morphology&&) = default;
Morphology& Morphology::operator=(Morphology&&) = default;

Morphology::~Morphology()
{
}

CellFamily Morphology::getCellFamily() const
{
    return _impl->plugin->getCellFamily();
}

Vector4fs& Morphology::getPoints()
{
    return _impl->plugin->getPoints();
}

const Vector4fs& Morphology::getPoints() const
{
    return _impl->plugin->getPoints();
}

Vector2is& Morphology::getSections()
{
    return _impl->plugin->getSections();
}

const Vector2is& Morphology::getSections() const
{
    return _impl->plugin->getSections();
}

SectionTypes& Morphology::getSectionTypes()
{
    return _impl->plugin->getSectionTypes();
}

const SectionTypes& Morphology::getSectionTypes() const
{
    return _impl->plugin->getSectionTypes();
}

floats& Morphology::getPerimeters()
{
    return _impl->plugin->getPerimeters();
}

const floats& Morphology::getPerimeters() const
{
    return _impl->plugin->getPerimeters();
}

MorphologyVersion Morphology::getVersion() const
{
    return _impl->plugin->getVersion();
}

const MorphologyInitData& Morphology::getInitData() const
{
    return _impl->plugin->getInitData();
}

}
