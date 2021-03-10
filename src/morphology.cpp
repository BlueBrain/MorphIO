#include <cassert>
#include <fstream>
#include <memory>
#include <streambuf>
#include <sstream>
#include <string>

#include <morphio/morphology.h>

#include <morphio/endoplasmic_reticulum.h>
#include <morphio/mito_section.h>
#include <morphio/mitochondria.h>
#include <morphio/mut/morphology.h>
#include <morphio/mut/section.h>
#include <morphio/mut/writers.h>
#include <morphio/shared_utils.tpp>
#include <morphio/soma.h>
#include <morphio/tools.h>


#include "readers/morphologyASC.h"
#include "readers/morphologyHDF5.h"
#include "readers/morphologySWC.h"


namespace morphio {

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



Morphology::Morphology(const Property::Properties& properties, unsigned int options)
    : TTree<Section, Morphology, morphio::mut::Morphology>(properties, options) {

    if (_properties->_cellLevel.fileFormat() != "swc")
        _properties->_cellLevel._somaType = getSomaType(soma().points().size());
}

Morphology::Morphology(const std::string& source, unsigned int options)
    : TTree<Section, Morphology, morphio::mut::Morphology>(source, options) {}

Morphology::Morphology(const HighFive::Group& group, unsigned int options)
    : TTree<Section, Morphology, morphio::mut::Morphology>(group, options) {}

Morphology::Morphology(morphio::mut::Morphology morphology)
    : TTree<Section, Morphology, morphio::mut::Morphology>(morphology) {}

Soma Morphology::soma() const {
    return Soma(_properties);
}

Mitochondria Morphology::mitochondria() const {
    return Mitochondria(_properties);
}

const EndoplasmicReticulum Morphology::endoplasmicReticulum() const {
    return EndoplasmicReticulum(_properties);
}

const std::vector<Property::Annotation>& Morphology::annotations() const {
    return _properties->_cellLevel._annotations;
}

const std::vector<Property::Marker>& Morphology::markers() const {
    return _properties->_cellLevel._markers;
}

const SomaType& Morphology::somaType() const {
    return _properties->somaType();
}


}  // namespace morphio
