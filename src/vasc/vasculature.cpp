#include <cassert>
#include <iostream>
#include <unistd.h>

#include <fstream>
#include <streambuf>

#include <morphio/vasc/vasculature.h>
#include <morphio/vasc/section.h>
#include <morphio/iterators.h>

#include "src/plugin/vasculatureHDF5.h"
#include "../plugin/morphologySWC.h"

namespace morphio {
namespace vasculature
{

void buildConnectivity(std::shared_ptr<property::Properties> properties);

Vasculature::Vasculature(const morphio::URI &source, unsigned int options)
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
            return plugin::h5::VasculatureHDF5().load(source);
        LBTHROW(UnknownFileType(
                "Unhandled file type"));
    };

    _properties = std::make_shared<property::Properties>(loader());

    buildConnectivity(_properties);
}

Vasculature::Vasculature(Vasculature&&) = default;
Vasculature& Vasculature::operator=(Vasculature&&) = default;

Vasculature::~Vasculature() {}

bool Vasculature::operator==(const Vasculature& other) const
{
    return this->_properties == other._properties;
}

bool Vasculature::operator!=(const morphio::vasculature::Vasculature& other) const
{
    return !this->operator==(other);
}

const Section Vasculature::section(const uint32_t& id) const
{
    return Section(id, _properties);
}

const std::vector<Section> Vasculature::sections() const
{
    std::vector<Section> sections;
    for (uint i = 0; i < _properties->get<property::VascSection>().size(); ++i) {
        sections.push_back(section(i));
    }
    return sections;
}

template <typename Property>
const std::vector<typename Property::Type>& Vasculature::get() const
{
    return _properties->get<Property>();
}

const Points& Vasculature::points() const
{
    return get<property::Point>();
}

const std::vector<float>& Vasculature::diameters() const
{
    return get<property::Diameter>();
}

const std::vector<property::SectionType::Type>& Vasculature::sectionTypes() const
{
    return get<property::SectionType>();
}

graph_iterator Vasculature::begin() const
{
    return graph_iterator(*this);
}

graph_iterator Vasculature::end() const
{
    return graph_iterator();
}

void buildConnectivity(std::shared_ptr<property::Properties> properties)
{

    //const auto& sections = properties->get<property::VascSection>();
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

} // namespace vasculature
} // namespace morphio
