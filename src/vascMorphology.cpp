#include <cassert>
#include <iostream>
#include <unistd.h>

#include <fstream>
#include <streambuf>

#include <morphio/vascMorphology.h>
#include <morphio/vascSection.h>

#include "plugin/vasc_morphologyHDF5.h"
#include "plugin/morphologySWC.h"

namespace morphio {
void buildConnectivity(std::shared_ptr<VasculatureProperty::Properties> properties);

VasculatureMorphology::VasculatureMorphology(const morphio::URI &source, unsigned int options)
{
    const size_t pos = source.find_last_of(".");
    if (pos == std::string::npos)
        LBTHROW(UnknownFileType("File has no extension"));

    if (access(source.c_str(), F_OK) == -1)
        LBTHROW(RawDataError("File: " + source + " does not exist."));

    std::string extension;

    for (auto& c : source.substr(pos))
        extension += std::tolower(c);

    auto loader = [&source, &options, &extension]() {
        if (extension == ".h5")
            return plugin::h5::load_vasc(source);
        //if (extension == ".swc")
        //    return plugin::swc::load(source, options);
        LBTHROW(UnknownFileType(
                "Unhandled file type"));
    };

    _properties = std::make_shared<VasculatureProperty::Properties>(loader());

    buildConnectivity(_properties);
}

VasculatureMorphology::VasculatureMorphology(VasculatureMorphology&&) = default;
VasculatureMorphology& VasculatureMorphology::operator=(VasculatureMorphology&&) = default;

VasculatureMorphology::~VasculatureMorphology() {}

bool VasculatureMorphology::operator==(const VasculatureMorphology& other) const
{
    if (this->_properties == other._properties)
        return true;
    else
        return false;
    //return (this->_properties && other._properties && *(this->_properties) == *(other._properties))
}

bool VasculatureMorphology::operator!=(const morphio::VasculatureMorphology& other) const
{
    return !this->operator==(other);
}

const VasculatureSection VasculatureMorphology::section(const uint32_t& id) const
{
    return VasculatureSection(id, _properties);
}
/*
const std::vector<VasculatureSection> VasculatureMorphology::rootSections() const
{
    std::vector<VasculatureSection> result;
    try {
        const std::map<uint32_t, std::vector<uint32_t>> section_map = _properties->predecessors<morphio::VasculatureProperty::VascSection>();
        const std::vector<int> sections = _properties->get<morphio::VasculatureProperty::VascSection>();
        std::cout << "sections read in " << sections.size() << std::endl;
        for (int i = 0 ; i < sections.size(); ++i) {
            if (section_map.find(sections[i]) == section_map.end())
                result.push_back(section(sections[i]));
        }
        return result;
    } catch (const std::out_of_range& oor) {
        return result;
    }
}
*/
const std::vector<VasculatureSection> VasculatureMorphology::sections() const
{
    std::vector<VasculatureSection> sections;
    for (uint i = 1; i < _properties->get<morphio::VasculatureProperty::VascSection>().size(); ++i) {
        sections.push_back(section(i));
    }
    return sections;
}

template <typename Property>
const std::vector<typename Property::Type>& VasculatureMorphology::get() const
{
    return _properties->get<Property>();
}

const Points& VasculatureMorphology::points() const
{
    return get<VasculatureProperty::Point>();
}

const std::vector<float>& VasculatureMorphology::diameters() const
{
    return get<VasculatureProperty::Diameter>();
}

const std::vector<SectionType>& VasculatureMorphology::sectionTypes() const
{
    return get<VasculatureProperty::SectionType>();
}

graph_iterator VasculatureMorphology::begin() const
{
    return graph_iterator(*this);
}

graph_iterator VasculatureMorphology::end() const
{
    return graph_iterator();
}

void buildConnectivity(std::shared_ptr<VasculatureProperty::Properties> properties)
{

    const auto& sections = properties->get<VasculatureProperty::VascSection>();
    const auto& connectivity = properties->get<VasculatureProperty::Connection>();
    auto& successors = properties->_sectionLevel._successors;
    auto& predecessors = properties->_sectionLevel._predecessors;

    for (size_t i = 0; i < connectivity.size(); ++i) {
        int32_t first = connectivity[i][0];
        int32_t second = connectivity[i][1];
        successors[first].push_back(second);
        predecessors[second].push_back(first);
    }

}

}