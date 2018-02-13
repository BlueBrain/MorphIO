#include <unistd.h>
#include <cassert>
#include <iostream>

#include <minimorph/morphology.h>
#include <minimorph/section.h>

#include "plugin/morphologyHDF5.h"
#include "plugin/morphologySWC.h"

namespace minimorph
{

Morphology::Morphology(const URI& source)
{
    const size_t pos = source.find_last_of(".");
    assert(pos != std::string::npos);
    if(access( source.c_str(), F_OK ) == -1)
        LBTHROW(RawDataError("File: "+source+" does not exist."));

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

const Soma Morphology::soma() const {
    return Soma(_properties);
}

const Section Morphology::section(const uint32_t& id) const {
    return Section(id, _properties);
}

const std::vector<Section> Morphology::rootSections() const {
    return soma().rootSections();
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
}
