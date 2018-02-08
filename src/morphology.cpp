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
    _properties = load_data(source);
}

Morphology::Morphology(Morphology&&) = default;
Morphology& Morphology::operator=(Morphology&&) = default;

Morphology::~Morphology()
{
}


Morphology::PropertiesPtr Morphology::load_data(const URI& uri)
{
    const size_t pos = uri.find_last_of(".");
    assert(pos != std::string::npos);
    if(access( uri.c_str(), F_OK ) == -1)
    {
        LBTHROW(RawDataError("File: "+uri+" does not exist."));
    }

    std::unique_ptr<MorphologyPlugin> plugin;
    if (uri.substr(pos) == ".h5") {
        plugin = std::unique_ptr<MorphologyPlugin>(new plugin::MorphologyHDF5(MorphologyInitData(uri)));
    }
    else if (uri.substr(pos) == ".swc") {
        plugin = std::unique_ptr<MorphologyPlugin>(new plugin::MorphologySWC(MorphologyInitData(uri)));
    }
    else {
        LBTHROW(UnknownFileType("unhandled file type"));
    }
    plugin -> extractInformation();

    return std::make_shared<Property::Properties>(plugin -> getProperties());
}


Sections Morphology::getSections(){
    Sections sections;
    for(int i = 0; i<_properties->get<minimorph::Property::Section>().size(); ++i){
        sections.push_back(Section(i, _properties));
    }
    return sections;
}


template <typename Property> typename Property::Type& Morphology::get(){
    return _properties->get<Property>();
}

template <typename Property> const typename Property::Type& Morphology::get() const{
    return _properties->get<Property>();
}

// Template method instantiations

template Property::Point::Type& Morphology::get<Property::Point>();
template const Property::Point::Type& Morphology::get<Property::Point>() const;

template Property::Diameter::Type& Morphology::get<Property::Diameter>();
template const Property::Diameter::Type& Morphology::get<Property::Diameter>() const;

template Property::Perimeter::Type& Morphology::get<Property::Perimeter>();
template const Property::Perimeter::Type& Morphology::get<Property::Perimeter>() const;

template Property::SectionType::Type& Morphology::get<Property::SectionType>();
template const Property::SectionType::Type& Morphology::get<Property::SectionType>() const;

template Property::Section::Type& Morphology::get<Property::Section>();
template const Property::Section::Type& Morphology::get<Property::Section>() const;
}
