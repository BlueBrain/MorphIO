#include <unistd.h>
#include <cassert>
#include <iostream>

#include <fstream>
#include <streambuf>

#include <minimorph/morphology.h>
#include <minimorph/section.h>
#include <minimorph/soma.h>

#include "plugin/morphologyHDF5.h"
#include "plugin/morphologySWC.h"
#include "plugin/morphologyASC.h"

namespace minimorph
{

Morphology::Morphology(const URI& source)
{
    const size_t pos = source.find_last_of(".");
    assert(pos != std::string::npos);
    if(access( source.c_str(), F_OK ) == -1)
        LBTHROW(RawDataError("File: "+source+" does not exist."));

    if (source.substr(pos) == ".h5") {
        _properties = std::make_shared<Property::Properties>(plugin::h5::load(source));
    }
    else if (source.substr(pos) == ".swc") {
        _properties = std::make_shared<Property::Properties>(plugin::swc::load(source));
    }
    else if (source.substr(pos) == ".asc") {
        _properties = std::make_shared<Property::Properties>(plugin::asc::load(source));
    }
    else {
        LBTHROW(UnknownFileType("unhandled file type"));
    }

    const auto& sections = _properties->get<Property::Section>();
    auto& children = _properties->_sectionLevel._children;

    for (size_t i = 0; i < sections.size(); ++i)
    {
        const int32_t parent = sections[i][1];
        if (parent != -1)
            children[parent].push_back(i);
    }
}

Morphology::Morphology(Morphology&&) = default;
Morphology& Morphology::operator=(Morphology&&) = default;

Morphology::~Morphology()
{
}

const Soma Morphology::soma() const {
    return Soma(_properties);
}

const Section Morphology::section(const uint32_t& id) const {
    return Section(id, _properties);
}

const std::vector<Section> Morphology::rootSections() const
{
    std::vector<Section> result;
    try {
        const std::vector<uint32_t>& children = _properties->children().at(0);
        result.reserve(children.size());
        for (const uint32_t id : children)
            result.push_back(section(id));
        return result;
    }
    catch (const std::out_of_range& oor) {
        return result;
    }
}

const std::vector<Section> Morphology::sections() const {
    std::vector<Section> sections;
    for(int i = 0; i<_properties->get<minimorph::Property::Section>().size(); ++i){
        sections.push_back(section(i));
    }
    return sections;
}

template <typename Property> const std::vector<typename Property::Type>& Morphology::get() const{
    return _properties->get<Property>();
}

const Points& Morphology::points() const { return get<Property::Point>(); }
const std::vector<float>& Morphology::diameters() const { return get<Property::Diameter>(); }
const std::vector<float>& Morphology::perimeters() const { return get<Property::Perimeter>(); }
const std::vector<SectionType>& Morphology::sectionTypes() const { return get<Property::SectionType>(); }
const CellFamily& Morphology::cellFamily() const { return _properties->cellFamily(); }
const MorphologyVersion& Morphology::version() const { return _properties->version(); }

} // namespace minimorph
