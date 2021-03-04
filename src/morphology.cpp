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

TMorphology::TMorphology(const Property::Properties& properties, unsigned int options)
    : _properties(std::make_shared<Property::Properties>(properties)) {
    buildChildren(_properties);

    if (_properties->_cellLevel.fileFormat() != "swc")
        _properties->_cellLevel._somaType = getSomaType(soma().points().size());

    // For SWC and ASC, sanitization and modifier application are already taken care of by
    // their respective loaders
    if (properties._cellLevel.fileFormat() == "h5") {
        mut::TMorphology<SectionType> mutable_morph(*this);
        mutable_morph.sanitize();
        if (options) {
            mutable_morph.applyModifiers(options);
        }
        _properties = std::make_shared<Property::Properties>(mutable_morph.buildReadOnly());
        buildChildren(_properties);
    }
}

TMorphology::TMorphology(const HighFive::Group& group, unsigned int options)
    : TMorphology(readers::h5::load(group), options) {}

TMorphology::TMorphology(const std::string& source, unsigned int options)
    : TMorphology(loadURI(source, options), options) {}


TMorphology::TMorphology(mut::TMorphology<SectionType> morphology) {
    morphology.sanitize();
    _properties = std::make_shared<Property::Properties>(morphology.buildReadOnly());
    buildChildren(_properties);
}

TMorphology::TMorphology(TMorphology&&) noexcept = default;
TMorphology& TMorphology::operator=(TMorphology&&) noexcept = default;

TMorphology::~TMorphology() = default;

Soma TMorphology::soma() const {
    return Soma(_properties);
}

Mitochondria TMorphology::mitochondria() const {
    return Mitochondria(_properties);
}

const EndoplasmicReticulum TMorphology::endoplasmicReticulum() const {
    return EndoplasmicReticulum(_properties);
}

const std::vector<Property::Annotation>& TMorphology::annotations() const {
    return _properties->_cellLevel._annotations;
}

const std::vector<Property::Marker>& TMorphology::markers() const {
    return _properties->_cellLevel._markers;
}

Section TMorphology::section(uint32_t id) const {
    return {id, _properties};
}

std::vector<Section> TMorphology::rootSections() const {
    std::vector<Section> result;
    try {
        const std::vector<uint32_t>& children =
            _properties->children<morphio::Property::Section>().at(-1);
        result.reserve(children.size());
        for (auto id : children) {
            result.push_back(section(id));
        }

        return result;
    } catch (const std::out_of_range&) {
        return result;
    }
}

std::vector<Section> TMorphology::sections() const {
    // TODO: Make this more performant when needed
    std::vector<Section> sections_;
    auto count = _properties->get<morphio::Property::Section>().size();
    sections_.reserve(count);
    for (unsigned int i = 0; i < count; ++i) {
        sections_.emplace_back(section(i));
    }
    return sections_;
}

template <typename Property>
const std::vector<typename Property::Type>& TMorphology::get() const {
    return _properties->get<Property>();
}

const Points& TMorphology::points() const noexcept {
    return get<Property::Point>();
}

std::vector<uint32_t> TMorphology::sectionOffsets() const {
    const std::vector<Property::Section::Type>& indices_and_parents = get<Property::Section>();
    auto size = indices_and_parents.size();
    std::vector<uint32_t> indices(size + 1);
    std::transform(indices_and_parents.begin(),
                   indices_and_parents.end(),
                   indices.begin(),
                   [](const Property::Section::Type& pair) { return pair[0]; });
    indices[size] = static_cast<uint32_t>(points().size());
    return indices;
}

const std::vector<morphio::floatType>& TMorphology::diameters() const {
    return get<Property::Diameter>();
}

const std::vector<morphio::floatType>& TMorphology::perimeters() const {
    return get<Property::Perimeter>();
}

const std::vector<SectionType>& TMorphology::sectionTypes() const {
    return get<Property::SectionType>();
}

const CellFamily& TMorphology::cellFamily() const {
    return _properties->cellFamily();
}

const SomaType& TMorphology::somaType() const {
    return _properties->somaType();
}

const std::map<int, std::vector<unsigned int>>& TMorphology::connectivity() const {
    return _properties->children<Property::Section>();
}

const MorphologyVersion& TMorphology::version() const {
    return _properties->version();
}

depth_iterator TMorphology::depth_begin() const {
    return depth_iterator(*this);
}

depth_iterator TMorphology::depth_end() const {
    return depth_iterator();
}

breadth_iterator TMorphology::breadth_begin() const {
    return breadth_iterator(*this);
}

breadth_iterator TMorphology::breadth_end() const {
    return breadth_iterator();
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
