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

#include "morphology.h"
#include <section.h>
#include "soma.h"

#include "morphologyImpl.h"

#include <morphology.h>

// #include <lunchbox/log.h>

namespace morphio
{
#if 0
Morphology::Morphology(const void* data, const size_t size)
    : _impl(new Impl(data, size))
{
}
#endif

Morphology::Morphology(const URI& source, const Matrix4f& transform)
    : _impl(new Impl(source, transform))
{
}

// Morphology::Morphology(const Morphology& from,
//                        const Matrix4f& transform)
//     : _impl(new Impl(from._impl->data, transform))
// {
// }

// Morphology::Morphology(const Morphology& from)
//     : _impl(new Impl(from._impl->data))
// {
// }

Morphology::Morphology(minimorph::MorphologyPtr morphology,
                       const Matrix4f& transform)
    : _impl(new Impl(morphology, transform))
{
}

Morphology::Morphology(const URI& source)
    : _impl(new Impl(source))
{
}

Morphology::Morphology(minimorph::ConstMorphologyPtr morphology)
    : _impl(new Impl(morphology))
{
}

Morphology::~Morphology()
{
}

const Points& Morphology::getPoints() const
{
    return _impl->data->getPoints();
}

const Vector2is& Morphology::getSections() const
{
    return _impl->data->getSections();
}

minimorph::MorphologyVersion Morphology::getVersion() const
{
    return _impl->data->getVersion();
}

const SectionTypes& Morphology::getSectionTypes() const
{
    return reinterpret_cast<const SectionTypes&>(
        _impl->data->getSectionTypes());
}

uint32_ts Morphology::getSectionIDs(const SectionTypes& types) const
{
    return _impl->getSectionIDs(types, false);
}

Sections Morphology::getSections(const SectionType type) const
{
    const SectionTypes types(1, type);
    const uint32_ts ids = _impl->getSectionIDs(types, true);
    Sections result;
    for (const uint32_t id : ids)
        result.push_back(Section(id, _impl));
    return result;
}

Sections Morphology::getSections(const SectionTypes& types) const
{
    const uint32_ts ids = _impl->getSectionIDs(types, true);
    Sections result;
    for (const uint32_t id : ids)
        result.push_back(Section(id, _impl));
    return result;
}

Section Morphology::getSection(const uint32_t& id) const
{
    auto& types = _impl->data->getSectionTypes();
    if (getSections().size() <= id || types.size() <= id)
        LBTHROW(std::runtime_error(std::string("Section ID ") +
                                   std::to_string(id) + " out of range"));

    if (types[id] == minimorph::enums::SECTION_SOMA)
        LBTHROW(std::runtime_error("The soma cannot be accessed as a Section"));

    return Section(id, _impl);
}

Soma Morphology::getSoma() const
{
    return Soma(_impl);
}

Sections Morphology::getRootSections() const
{
    return getSoma().getChildren();
}

const Matrix4f& Morphology::getTransformation() const
{
    return _impl->transformation;
}

// TODO: fix me
// servus::Serializable::Data Morphology::toBinary() const
// {
//     return _impl->data->toBinary();
// }
}
