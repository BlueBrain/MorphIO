#include <cassert>
#include <iostream>
#include <unistd.h>

#include <fstream>
#include <streambuf>

#include <morphio/iterators.h>
#include <morphio/vasc/section.h>
#include <morphio/vasc/vasculature.h>

#include "../plugin/morphologySWC.h"
#include "src/plugin/vasculatureHDF5.h"

namespace morphio {
namespace vasculature {

void buildConnectivity(std::shared_ptr<property::Properties> properties);

Vasculature::Vasculature(const std::string& source)
{
    const size_t pos = source.find_last_of(".");
    if (pos == std::string::npos)
        LBTHROW(UnknownFileType("File has no extension"));

    if (access(source.c_str(), F_OK) == -1)
        LBTHROW(RawDataError("File: " + source + " does not exist."));

    std::string extension = source.substr(pos);

    property::Properties loader;
    if (extension == ".h5")
        loader = plugin::h5::VasculatureHDF5().load(source);
    else
        LBTHROW(UnknownFileType("File: " + source + " does not end with the .h5 extension"));

    _properties = std::make_shared<property::Properties>(loader);

    buildConnectivity(_properties);
}

Vasculature::Vasculature(Vasculature&&) = default;
Vasculature& Vasculature::operator=(Vasculature&&) = default;

Vasculature::~Vasculature()
{
}

bool Vasculature::operator==(const Vasculature& other) const
{
    if (this->_properties == other._properties)
        return true;
    return (this->_properties && other._properties && *(this->_properties) == *(other._properties));
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
    std::vector<Section> sections_;
    for (uint i = 0; i < _properties->get<property::VascSection>().size(); ++i) {
        sections_.push_back(section(i));
    }
    return sections_;
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
    const std::vector<std::array<unsigned int, 2>>& connectivity = properties->get<property::Connection>();
    std::map<uint32_t, std::vector<uint32_t>>& successors = properties->_sectionLevel._successors;
    std::map<uint32_t, std::vector<uint32_t>>& predecessors = properties->_sectionLevel._predecessors;

    for (size_t i = 0; i < connectivity.size(); ++i) {
        uint32_t first = connectivity[i][0];
        uint32_t second = connectivity[i][1];
        successors[first].push_back(second);
        predecessors[second].push_back(first);
    }
}

} // namespace vasculature
} // namespace morphio
