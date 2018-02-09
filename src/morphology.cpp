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

#include <unistd.h>

#include "morphology.h"

/* todo: compile */
#include <cassert>
#include <iostream>


#include "plugin/morphologyHDF5.h"
#include "plugin/morphologySWC.h"
#include <section.h>

namespace minimorph
{

Morphology::Morphology(const URI& source)
{
    const size_t pos = source.find_last_of(".");
    assert(pos != std::string::npos);
    if(access( source.c_str(), F_OK ) == -1)
    {
        LBTHROW(RawDataError("File: "+source+" does not exist."));
    }

    std::unique_ptr<MorphologyPlugin> plugin;
    if (source.substr(pos) == ".h5") {
        plugin = std::unique_ptr<MorphologyPlugin>(new plugin::MorphologyHDF5(MorphologyInitData(source)));
    }
    else if (source.substr(pos) == ".swc") {
        plugin = std::unique_ptr<MorphologyPlugin>(new plugin::MorphologySWC(MorphologyInitData(source)));
    }
    else {
        LBTHROW(UnknownFileType("unhandled file type"));
    }
    plugin -> extractInformation();

    _properties = std::make_shared<Property::Properties>(plugin -> getProperties());
}

Morphology::Morphology(Morphology&&) = default;
Morphology& Morphology::operator=(Morphology&&) = default;

Morphology::~Morphology()
{
}

Soma Morphology::getSoma() const {
    return Soma(_properties);
}

Section Morphology::getSection(const uint32_t& id) const {
    return Section(id, _properties);
}

Sections Morphology::getSections(){
    Sections sections;
    for(int i = 0; i<_properties->get<minimorph::Property::Section>().size(); ++i){
        sections.push_back(getSection(i));
    }
    return sections;
}


template <typename Property> typename Property::Type& Morphology::get(){
    return _properties->get<Property>();
}

template <typename Property> const typename Property::Type& Morphology::get() const{
    return _properties->get<Property>();
}

const Points Morphology::getPoints() const { return get<Property::Point>(); }
const floats Morphology::getDiameters() const { return get<Property::Diameter>(); }
const floats Morphology::getPerimeters() const { return get<Property::Perimeter>(); }
const SectionTypes Morphology::getSectionTypes() const { return get<Property::SectionType>(); }
const CellFamily Morphology::getCellFamily() const { return get<Property::CellFamily>(); }



}
