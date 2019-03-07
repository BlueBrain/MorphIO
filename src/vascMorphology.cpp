#include <cassert>
#include <iostream>
#include <unistd.h>

#include <ftsream>
#include <streambuf>

#include <morphio/vascMorphology.h>
#include <morphio/vascSection.h>

#include "plugin/morphologyHDF5.h"

namespace morphio {

Vasculature Morphology::VasculatureMorphology(const URI& source, unsigned int options)
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
            return plugin::h5::load(source);
        LBTHROW(UnknownFileType(
            "Unhandles file type: only H5 is supported"));
    };

    _properties = std::make_shared<VasculatureProperty::Properties>(loader());
}

VasculatureMorphology::VasculatureMorphology(VasculatureMorphology&&) = default;
VasculatureMorphology& VasculatureMorphology::operator=(VasculatureMorphology&&) = default;

VasculatureMorphology::~VasculatureMorphology() {}

bool VasculatureMorphology::operator==(const VasculatureMorphology& other) const
{
    if (this->_properties == other._properties)
        return true;

    return (this->_properties && other._properties && *(this->_properties) == *(other._properties));
}

bool VasculatureMorphology::operator!=(const VasculatureMorphology& other) const
{
    return !this->operator==(other);
}

const VasculatureSection VasculatureMorphology::section(const uint32_t& id) const
{
    return VasculatureSection(id, _properties);
}

const std::vector<VasculatureSection> VasculatureMorphology::rootSections() const
{
    std::vector<VasculatureSection> result;
    try {
        const std::vector<uint32_t>& neighbors = _properties->neighbors<morphio::VasculatureProperty::VasculatureSection>().at(-1);
        re
