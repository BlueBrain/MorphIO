#include <cassert>
#include <iostream>
#include <unistd.h>

#include <fstream>
#include <streambuf>

#include <morphio/vasc/morphology.h>
#include <morphio/vasc/section.h>
#include <morphio/iterators.h>

#include "../plugin/vasc_morphologyHDF5.h"
#include "../plugin/morphologySWC.h"

namespace morphio {
namespace vasculature
{

void buildConnectivity(std::shared_ptr<property::Properties> properties);

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
            return plugin::h5::VasculatureMorphologyHDF5().load(source);
        LBTHROW(UnknownFileType(
                "Unhandled file type"));
    };

    _properties = std::make_shared<property::Properties>(loader());

    buildConnectivity(_properties);
}

VasculatureMorphology::VasculatureMorphology(VasculatureMorphology&&) = default;
VasculatureMorphology& VasculatureMorphology::operator=(VasculatureMorphology&&) = default;

VasculatureMorphology::~VasculatureMorphology() {}

bool VasculatureMorphology::operator==(const VasculatureMorphology& other) const
{
    return this->_properties == other._properties;
}

bool VasculatureMorphology::operator!=(const morphio::vasculature::VasculatureMorphology& other) const
{
    return !this->operator==(other);
}

const VasculatureSection VasculatureMorphology::section(const uint32_t& id) const
{
    return VasculatureSection(id, _properties);
}

const std::vector<VasculatureSection> VasculatureMorphology::sections() const
{
    std::vector<VasculatureSection> sections;
    for (uint i = 0; i < _properties->get<property::VascSection>().size(); ++i) {
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
    return get<property::Point>();
}

const std::vector<float>& VasculatureMorphology::diameters() const
{
    return get<property::Diameter>();
}

const std::vector<property::SectionType::Type>& VasculatureMorphology::sectionTypes() const
{
    return get<property::SectionType>();
}

graph_iterator VasculatureMorphology::begin() const
{
    return graph_iterator(*this);
}

graph_iterator VasculatureMorphology::end() const
{
    return graph_iterator();
}

void buildConnectivity(std::shared_ptr<property::Properties> properties)
{

    const auto& sections = properties->get<property::VascSection>();
    const auto& connectivity = properties->get<property::Connection>();
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
}