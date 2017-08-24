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

#include <lunchbox/plugin.h>
#include <lunchbox/pluginFactory.h>
#include <lunchbox/threadPool.h>

namespace brion
{
namespace
{
lunchbox::ThreadPool& _getWorkers()
{
    static lunchbox::ThreadPool workers;
    return workers;
}

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
        if (!fromBinary(data, size))
            LBTHROW(std::runtime_error(
                "Failed to construct morphology from binary data"));
    }

    void load() final { /*NOP*/}
};
}

class Morphology::Impl
{
public:
    typedef lunchbox::PluginFactory<MorphologyPlugin> MorphologyPluginFactory;

    explicit Impl(const MorphologyInitData& initData)
        : plugin(MorphologyPluginFactory::getInstance().create(initData))
    {
        loadFuture = _getWorkers().post([&] {
            plugin->load();
            if (plugin->getPoints().empty())
                LBTHROW(std::runtime_error(
                    "Failed to load morphology " +
                    std::to_string(plugin->getInitData().getURI())));
        });
    }

    Impl(const Morphology& from)
        : plugin(new BinaryMorphology(from))
    {
    }

    Impl(const void* data, size_t size)
        : plugin(new BinaryMorphology(data, size))
    {
    }

    ~Impl()
    {
        try
        {
            finishLoad();
        }
        catch (const std::exception& e)
        {
            LBERROR << e.what() << std::endl;
        }
        catch (...)
        {
            LBERROR << "Unknown exception during morphology load" << std::endl;
        }
    }

    void finishLoad() const
    {
        std::lock_guard<std::mutex> lock(futureMutex);
        if (!loadFuture.valid())
            return;

        loadFuture.get();
    }

    std::unique_ptr<MorphologyPlugin> plugin;
    mutable std::future<void> loadFuture; // fulfilled by worker thread pool
    mutable std::mutex futureMutex;
};

Morphology::Morphology(const URI& source)
    : _impl(new Impl(MorphologyInitData(source)))
{
}

Morphology::Morphology(const void* data, size_t size)
    : _impl(new Impl(data, size))
{
}

Morphology::Morphology(const Morphology& from)
    : _impl(new Impl(from))
{
}

Morphology& Morphology::operator=(const Morphology& from)
{
    if (this != &from)
        _impl.reset(new Impl(from));
    return *this;
}

Morphology::Morphology(Morphology&&) = default;
Morphology& Morphology::operator=(Morphology&&) = default;

Morphology::~Morphology()
{
}

CellFamily Morphology::getCellFamily() const
{
    _impl->finishLoad();
    return _impl->plugin->getCellFamily();
}

Vector4fs& Morphology::getPoints()
{
    _impl->finishLoad();
    return _impl->plugin->getPoints();
}

const Vector4fs& Morphology::getPoints() const
{
    _impl->finishLoad();
    return _impl->plugin->getPoints();
}

Vector2is& Morphology::getSections()
{
    _impl->finishLoad();
    return _impl->plugin->getSections();
}

const Vector2is& Morphology::getSections() const
{
    _impl->finishLoad();
    return _impl->plugin->getSections();
}

SectionTypes& Morphology::getSectionTypes()
{
    _impl->finishLoad();
    return _impl->plugin->getSectionTypes();
}

const SectionTypes& Morphology::getSectionTypes() const
{
    _impl->finishLoad();
    return _impl->plugin->getSectionTypes();
}

floats& Morphology::getPerimeters()
{
    _impl->finishLoad();
    return _impl->plugin->getPerimeters();
}

const floats& Morphology::getPerimeters() const
{
    _impl->finishLoad();
    return _impl->plugin->getPerimeters();
}

MorphologyVersion Morphology::getVersion() const
{
    _impl->finishLoad();
    return _impl->plugin->getVersion();
}

const MorphologyInitData& Morphology::getInitData() const
{
    _impl->finishLoad();
    return _impl->plugin->getInitData();
}

servus::Serializable::Data Morphology::toBinary() const
{
    _impl->finishLoad();
    return _impl->plugin->toBinary();
}
}
