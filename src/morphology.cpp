/* Copyright (c) 2013-2023, EPFL/Blue Brain Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <cctype>  // std::tolower
#include <fstream>
#include <iterator>  // std::back_inserter
#include <memory>

#include <morphio/endoplasmic_reticulum.h>
#include <morphio/mitochondria.h>
#include <morphio/morphology.h>
#include <morphio/section.h>
#include <morphio/soma.h>

#include <morphio/mut/morphology.h>

#include "readers/morphologyASC.h"
#include "readers/morphologyHDF5.h"
#include "readers/morphologySWC.h"

namespace {

std::string readCompleteFile(const std::string& path) {
    std::ifstream ifs(path);

    if (!ifs) {
        throw(morphio::RawDataError("File: " + path + " does not exist."));
    }

    std::ostringstream oss;
    oss << ifs.rdbuf();

    return oss.str();
}

void buildChildren(const std::shared_ptr<morphio::Property::Properties>& properties) {
    {
        const auto& sections = properties->get<morphio::Property::Section>();
        auto& children = properties->_sectionLevel._children;

        for (unsigned int i = 0; i < sections.size(); ++i) {
            const int32_t parent = sections[i][1];
            children[parent].push_back(i);
        }
    }

    {
        const auto& sections = properties->get<morphio::Property::MitoSection>();
        auto& children = properties->_mitochondriaSectionLevel._children;

        for (unsigned int i = 0; i < sections.size(); ++i) {
            const int32_t parent = sections[i][1];
            children[parent].push_back(i);
        }
    }
}

std::string tolower(const std::string& str) {
    std::string ret;
    std::transform(str.begin(), str.end(), std::back_inserter(ret), [](unsigned char c) {
        return std::tolower(c);
    });
    return ret;
}

morphio::Property::Properties loadFile(const std::string& path, unsigned int options) {
    const size_t pos = path.find_last_of('.');
    if (pos == std::string::npos || pos == path.length() - 1) {
        throw(morphio::UnknownFileType("File has no extension"));
    }

    std::string extension = tolower(path.substr(pos + 1));

    if (extension == "h5") {
        return morphio::readers::h5::load(path);
    } else if (extension == "asc") {
        std::string contents = readCompleteFile(path);
        return morphio::readers::asc::load(path, contents, options);
    } else if (extension == "swc") {
        std::string contents = readCompleteFile(path);
        return morphio::readers::swc::load(path, contents, options);
    }

    throw(morphio::UnknownFileType("Unhandled file type: '" + extension +
                                   "' only SWC, ASC and H5 are supported"));
}


morphio::Property::Properties loadString(const std::string& contents,
                                         const std::string& extension,
                                         unsigned int options) {
    std::string lower_extension = tolower(extension);

    if (lower_extension == "asc") {
        return morphio::readers::asc::load("$STRING$", contents, options);
    } else if (lower_extension == "swc") {
        return morphio::readers::swc::load("$STRING$", contents, options);
    }

    throw(morphio::UnknownFileType("Unhandled file type: '" + lower_extension +
                                   "' only SWC, ASC and H5 are supported"));
}

}  // namespace

namespace morphio {

Morphology::Morphology(const Property::Properties& properties, unsigned int options)
    : properties_(std::make_shared<Property::Properties>(properties)) {
    buildChildren(properties_);

    // For SWC and ASC, sanitization and modifier application are already taken care of by
    // their respective loaders
    if (properties._cellLevel.fileFormat() == "h5" && options) {
        mut::Morphology mutable_morph(*this);
        mutable_morph.applyModifiers(options);
        properties_ = std::make_shared<Property::Properties>(mutable_morph.buildReadOnly());
        buildChildren(properties_);
    }
}

Morphology::Morphology(const std::string& path, unsigned int options)
    : Morphology(loadFile(path, options), options) {}

Morphology::Morphology(const HighFive::Group& group, unsigned int options)
    : Morphology(readers::h5::load(group), options) {}

Morphology::Morphology(const mut::Morphology& morphology) {
    properties_ = std::make_shared<Property::Properties>(morphology.buildReadOnly());
    buildChildren(properties_);
}

Morphology::Morphology(const std::string& contents,
                       const std::string& extension,
                       unsigned int options)
    : Morphology(loadString(contents, extension, options), options) {}

Soma Morphology::soma() const {
    return Soma(properties_);
}

Mitochondria Morphology::mitochondria() const {
    return Mitochondria(properties_);
}

EndoplasmicReticulum Morphology::endoplasmicReticulum() const {
    return EndoplasmicReticulum(properties_);
}

const std::vector<Property::Annotation>& Morphology::annotations() const {
    return properties_->_cellLevel._annotations;
}

const std::vector<Property::Marker>& Morphology::markers() const {
    return properties_->_cellLevel._markers;
}

Section Morphology::section(uint32_t id) const {
    return {id, properties_};
}

std::vector<Section> Morphology::rootSections() const {
    const auto& sections = properties_->children<morphio::Property::Section>();

    if (sections.empty()) {
        return {};
    }

    std::vector<Section> result;

    const std::vector<uint32_t>& children = sections.at(-1);
    result.reserve(children.size());
    for (auto id : children) {
        result.push_back(section(id));
    }

    return result;
}

std::vector<Section> Morphology::sections() const {
    // TODO: Make this more performant when needed
    std::vector<Section> sections_;
    auto count = properties_->get<morphio::Property::Section>().size();
    sections_.reserve(count);
    for (unsigned int i = 0; i < count; ++i) {
        sections_.emplace_back(section(i));
    }
    return sections_;
}

template <typename Property>
const std::vector<typename Property::Type>& Morphology::get() const {
    return properties_->get<Property>();
}

const Points& Morphology::points() const noexcept {
    return get<Property::Point>();
}

std::vector<uint32_t> Morphology::sectionOffsets() const {
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

const std::vector<morphio::floatType>& Morphology::diameters() const {
    return get<Property::Diameter>();
}

const std::vector<morphio::floatType>& Morphology::perimeters() const {
    return get<Property::Perimeter>();
}

const std::vector<SectionType>& Morphology::sectionTypes() const {
    return get<Property::SectionType>();
}

const CellFamily& Morphology::cellFamily() const {
    return properties_->cellFamily();
}

const SomaType& Morphology::somaType() const {
    return properties_->somaType();
}

const std::map<int, std::vector<unsigned int>>& Morphology::connectivity() const {
    return properties_->children<Property::Section>();
}

const MorphologyVersion& Morphology::version() const {
    return properties_->version();
}

depth_iterator Morphology::depth_begin() const {
    return depth_iterator(*this);
}

depth_iterator Morphology::depth_end() const {
    return depth_iterator();
}

breadth_iterator Morphology::breadth_begin() const {
    return breadth_iterator(*this);
}

breadth_iterator Morphology::breadth_end() const {
    return breadth_iterator();
}

}  // namespace morphio
