#include <cassert>
#include <iostream>

#include <fstream>
#include <memory>
#include <streambuf>

#include <morphio/endoplasmic_reticulum.h>
#include <morphio/mitochondria.h>
#include <morphio/morphology.h>
#include <morphio/section.h>
#include <morphio/soma.h>
#include <morphio/tools.h>

#include <morphio/mut/morphology.h>

#include "readers/morphologyASC.h"
#include "readers/morphologyHDF5.h"
#include "readers/morphologySWC.h"

namespace morphio {
void buildChildren(std::shared_ptr<Property::Properties> properties);
SomaType getSomaType(long unsigned int nSomaPoints);
Property::Properties loadURI(const std::string& source, unsigned int options);


SomaType getSomaType(long unsigned int nSomaPoints) {
    try {
        return std::map<long unsigned int, SomaType>{{0, SOMA_UNDEFINED},
                                                     {1, SOMA_SINGLE_POINT},
                                                     {2, SOMA_UNDEFINED}}
            .at(nSomaPoints);
    } catch (const std::out_of_range&) {
        return SOMA_SIMPLE_CONTOUR;
    }
}

void buildChildren(std::shared_ptr<Property::Properties> properties) {
    {
        const auto& sections = properties->get<Property::Section>();
        auto& children = properties->_sectionLevel._children;

        for (unsigned int i = 0; i < sections.size(); ++i) {
            const int32_t parent = sections[i][1];
            children[parent].push_back(i);
        }
    }

    {
        const auto& sections = properties->get<Property::MitoSection>();
        auto& children = properties->_mitochondriaSectionLevel._children;

        for (unsigned int i = 0; i < sections.size(); ++i) {
            const int32_t parent = sections[i][1];
            children[parent].push_back(i);
        }
    }
}

Property::Properties loadURI(const std::string& source, unsigned int options) {
    const size_t pos = source.find_last_of(".");
    if (pos == std::string::npos)
        throw(UnknownFileType("File has no extension"));

    // Cross-platform check of file existance
    std::ifstream file(source.c_str());
    if (!file) {
        throw(RawDataError("File: " + source + " does not exist."));
    }

    std::string extension = source.substr(pos);

    auto loader = [&source, &options, &extension]() {
        if (extension == ".h5" || extension == ".H5")
            return readers::h5::load(source);
        if (extension == ".asc" || extension == ".ASC")
            return readers::asc::load(source, options);
        if (extension == ".swc" || extension == ".SWC")
            return readers::swc::load(source, options);
        throw(UnknownFileType("Unhandled file type: only SWC, ASC and H5 are supported"));
    };

    return loader();
}

}  // namespace morphio
